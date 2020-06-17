#!/usr/bin/python3
#
# OLC - Oxidian-Lang Compiler [V1.2.0]

from sys import argv, version_info, exit, stdout, platform
from struct import pack, unpack
from struct import calcsize as scalcsize
from binascii import hexlify, unhexlify
from time import time
import string, json, os

print("Oxidian-Lang Compiler [Version 1.2.0] - Everything else is irrelevant.\n")

if version_info[0] != 3:
    print("OLC is intended to run with Python 3.* since Python 2 is deprecated.")
    exit(1)

if len(argv) < 2:
    print("Missing input file.")
    exit(1)

class environment:
    verbose             = False
    variables           = {}
    labels              = {}
    macros              = {}
    instructions        = []
    enddata             = 0
    endcode             = 0
    endpreprocessor     = 0
    current_instruction = 0
    stacksize           = b"\xff\xff\xff\xff"
    options             = 0
    custom_datasec_addr = False
    custom_codesec_addr = False
    datasec_addr        = 0
    codesec_addr        = 0
    include_paths       = []
    output_filename     = "out.oxy"

try:
    environment.include_paths.append(os.environ["OXY_LIBS"])
except KeyError:
    if platform.startswith("linux"):
        if os.path.isdir("/usr/local/OxyLibs"):
            environment.include_paths.append("/usr/local/OxyLibs")
    elif platform.startswith("win32"):
        if os.path.isdir(os.environ["LOCALAPPDATA"]+"\\OxyLibs"):
            environment.include_paths.append(os.environ["LOCALAPPDATA"]+"\\OxyLibs")

for arg in argv:
    if arg.startswith("--output=") or arg.startswith("-o="):
        environment.output_filename = "=".join(npart for npart in arg.split("=")[1:])
    elif arg == "-v" or arg == "--verbose":
        environment.verbose = True
    elif arg.startswith("--include=") or arg.startswith("-i="):
        environment.include_paths += "=".join(npart for npart in arg.split("=")[1:]).split(";")

if environment.verbose:
    print("Include paths: %s" % str(environment.include_paths))

libraries = {
    # name,id
    'IO':     b'\x00\x00',
    'MEMORY': b'\x01\x00',
    'NET':    b'\x02\x00'
}

instructions = {
    # name,size,number,arg_struct
    'loadlib': (2,b'\x00\x00',["library"]),
    'setarg':  (5,b'\x01\x00',["byte", "variable"]),
    'call':    (2,b'\x02\x00',["short"]),
    'goto':    (4,b'\x03\x00',["instint/label"]),
    'add':     (8,b'\x04\x00',["variable","int"]),
    'sub':     (8,b'\x05\x00',["variable","int"]),
    'cmp':     (8,b'\x06\x00',["variable","int"]),
    'cl':      (0,b'\x07\x00',[]),
    'ch':      (0,b'\x08\x00',[]),
    'ce':      (0,b'\x09\x00',[]),
    'inc':     (4,b'\x0a\x00',["variable"]),
    'dec':     (4,b'\x0b\x00',["variable"]),
    'ipsh':    (4,b'\x0c\x00',["variable"]),
    'psh':     (2,b'\x0d\x00',["short"]),
    'pop':     (8,b'\x0e\x00',["variable", "int"]),
    'sspp':    (4,b'\x0f\x00',["int"]),
    'igetl':   (8,b'\x10\x00',["variable","variable"]),
    'isspp':   (4,b'\x11\x00',["variable"]),
    'iadd':    (8,b'\x12\x00',["variable","variable"]),
    'isub':    (8,b'\x13\x00',["variable","variable"]),
    'icmp':    (8,b'\x14\x00',["variable","variable"]),
    'and':     (8,b'\x15\x00',["variable","int"]),
    'iand':    (8,b'\x16\x00',["variable","variable"]),
    'or':      (8,b'\x17\x00',["variable","int"]),
    'ior':     (8,b'\x18\x00',["variable","variable"]),
    'xor':     (8,b'\x19\x00',["variable","int"]),
    'ixor':    (8,b'\x1a\x00',["variable","variable"]),
    'iref':    (8,b'\x1b\x00',["variable","variable"]),
    'ipop':    (8,b'\x1c\x00',["variable","variable"]),
    'invoke':  (4,b'\x1d\x00',["instint/label"]),
    'return':  (0,b'\x1e\x00',[]),
    'gspp':    (4,b'\x1f\x00',["variable"]),
    'cne':     (0,b'\x20\x00',[]),
    'cnh':     (0,b'\x21\x00',[]),
    'cnl':     (0,b'\x22\x00',[]),
    'chkerr':  (4,b'\x23\x00',["int"]),
    'eof':     (0,b'\xff\xff',[])
}

def tryconvert(source, target):
    try:
        target(source)
        return True
    except ValueError:
        return False
    except TypeError:
        return False
    except Exception as e:
        print("Error occoured while trying to convert value. Error: %s" % str(e))
        exit(1)

def get_dbglen(s):
    return (unpack("I", s[0:4])[0], s[4:])
def get_dbgstr(s,l):
    return (s[0:l], s[l:])

def verifyLabel(name):
    if name[0] not in string.ascii_letters:
        return False
    for c in name:
        if c not in string.ascii_letters+string.digits:
            return False
    return True

def calcsize(instruction_list):
    size = 0
    for index,instruction,args in instruction_list:
        size += instructions[instruction][0]+2
    return size

def transform_path(path):
    if platform.startswith("win32"):
        return path.replace("/","\\")
    else:
        return path

def get_library(name):
    for include_path in environment.include_paths:
        path = transform_path(include_path+"/"+name)
        if os.path.isfile(path):
            return path
    return False

def ascii2int(value):
    if value.startswith("ASCII."):
        if len(value[6:]) == 4:
            return True
        else:
            return False
    else:
        return False

def compile_variables(variables):
    comp = b""
    for reserve, length, variable in variables.values():
        try:
            comp += pack("I", length)+(b"\x01" if reserve else b"\x00")+unhexlify(variable)
        except Exception as e:
            print("Compilation failed while compiling variable (%s), error: %s" % (variable, str(e)))
            exit(1)
    return comp

def remove_whitespaces(vstring):
    for whitespace in string.whitespace:
        vstring = vstring.replace(whitespace, "")
    return vstring

def check_macro(line, macro):
    line = "\n{}\n".format(line)
    if macro in line:
        if line[line.find(macro)-1] in string.whitespace and line[line.find(macro)+len(macro)] in string.whitespace:
            # if there are whitespace before and after the macro name
            return True
        else:
            return False
    return False

def parse_string(string, code=False, resv=False):
    if string.startswith("ASCII."):
        return hexlify(string[6:].encode()).decode()
    elif string.startswith("TIMES(") and string.endswith(")") and tryconvert(string[6:].split(",")[0].strip(), int):
        try:
            times = int(string[6:].split(",")[0].strip())
            value = ",".join(value_part for value_part in string[6:].split(",")[1:])[:-1].strip()
            value = parse_string(value)*times
        except RecursionError:
            print("To much recursion while handling TIMES macro.")
            exit(1)
        except Exception as e:
            print("Error while handling TIMES macro. Error: %s" % str(e))
            exit(1)
        return value
    elif resv and string.startswith("resv(") and string.endswith(")") and tryconvert(string[5:][:-1].strip(), int):
        if int(string[5:][:-1].strip()) <= 4294967295:
            return int(string[5:][:-1].strip())
        else:
            print("Error while handling resv macro. Number too big. (>2^32)")
            exit(1)
    elif string.startswith("POINTER(") and string.endswith(")") and tryconvert(string[8:][:-1].strip(), int):
        if int(string[8:][:-1].strip()) <= unpack("P", b'\xff'*scalcsize("P"))[0]:
            try:
                return hexlify(pack("P", int(string[8:][:-1].strip()))).decode()
            except Exception as e:
                print("Error while handling POINTER macro. Error: %s" % str(e))
                exit(1)
        else:
            print("Error while handling POINTER macro. Number is too big. (>2^%d)" % scalcsize("P")*8)
            exit(1)
    elif string.startswith("LONG(") and string.endswith(")") and tryconvert(string[5:][:-1].strip(), int):
        if int(string[5:][:-1].strip()) <= 18446744073709551615:
            try:
                return hexlify(pack("L", int(string[5:][:-1].strip()))).decode()
            except Exception as e:
                print("Error while handling LONG macro. Error: %s" % str(e))
                exit(1)
        else:
            print("Error while handling LONG macro. Number is too big. (>2^64)")
            exit(1)
    elif string.startswith("INT(") and string.endswith(")") and tryconvert(string[4:][:-1].strip(), int):
        if int(string[4:][:-1].strip()) <= 4294967295:
            try:
                return hexlify(pack("I", int(string[4:][:-1].strip()))).decode()
            except Exception as e:
                print("Error while handling INT macro. Error: %s" % str(e))
                exit(1)
        else:
            print("Error while handling INT macro. Number too big. (>2^32)")
            exit(1)
    elif string.startswith("SHORT(") and string.endswith(")") and tryconvert(string[6:][:-1].strip(), int):
        if int(string[6:][:-1].strip()) <= 65535:
            try:
                return hexlify(pack("H", int(string[6:][:-1].strip()))).decode()
            except Exception as e:
                print("Error while handling SHORT macro. Error: %s" % str(e))
                exit(1)
        else:
            print("Error while handling SHORT macro. Number too big. (>65535)")
            exit(1)
    elif string.startswith("BYTE(") and string.endswith(")") and tryconvert(string[5:][:-1].strip(), int):
        if int(string[5:][:-1].strip()) <= 256:
            try:
                return hexlify(pack("B", int(string[5:][:-1].strip()))).decode()
            except Exception as e:
                print("Error while handling BYTE macro. Error: %s" % str(e))
                exit(1)
        else:
            print("Error while handling BYTE macro. Number too big. (>256)")
            exit(1)
    elif string in list(environment.variables.keys()) and not code:
        return environment.variables[string]
    elif tryconvert(string, unhexlify):
        return string
    elif not code:
        print("Unidentified value type: %s" % string)
        exit(1)
    else:
        return string

def parse_instruction(line):
    parts = line.strip().split(" ")
    instruction = parts[0].lower()
    arguments_ = parts[1:]
    arguments = []
    for argument in arguments_:
        arguments.append(parse_string(argument, code=True))
    parsed = (environment.current_instruction, instruction, arguments)
    environment.instructions.append(parsed)

### START ###
time_start = time()
stdout.write("Reading Input File... ")
fp = open(argv[1], "r")
content = fp.read()
fp.close()
content = [line.strip() for line in content.split("\n")]
content = [line if not line.startswith(";") else "" for line in content]
print("OK.")
stdout.write("Parsing File Sections... ")

### PARSING ###
for index, line in enumerate(content):
    if remove_whitespaces(line).lower() == "preprocessor{":
        for index_, line_ in enumerate(content[index+1:]):
            if line_ and not remove_whitespaces(line_) == "}":
                line_ = line_.strip()
                #########
                if line_.lower().startswith("including"):
                    including = line_.split(" ")[0][9:]
                    if including.startswith("[") and including.endswith("]"):
                        including = including[1:][:-1].split(",")
                    else:
                        including = False
                    filename = " ".join(fpart for fpart in line_.split(" ")[1:])
                    if not filename: # in case there is no space
                        filename = line_[7:]
                        if not filename:
                            print("Please provide a filename when including...")
                            exit(1)
                    if filename.startswith("<") and filename.endswith(">"): # include from include path
                        filename = filename[1:-1]
                        library = get_library(filename)
                        if not library:
                            print("Cannot find library '%s'." % filename)
                            exit(1)
                        filename = library
                    elif filename.startswith("\"") and filename.endswith("\""): # include from relative path
                        filename = filename[1:-1]
                        if not os.path.isfile(filename):
                            print("Cannot find library '%s'." % filename)
                            exit(1)
                    else:
                        print("Invalid include statement: %s" % filename)
                        exit(1)

                    with open(filename, "r") as fp:
                        try:
                            filedata = json.load(fp)
                            if including:
                                for name in including:
                                    if name in list(filedata["PREPROCESSOR"].keys()):
                                        environment.macros[name] = "\n".join(filedata["PREPROCESSOR"][name])
                                    elif name in list(filedata["DATA"].keys()):
                                        length = 0
                                        reserve = False
                                        value_ = filedata["DATA"][name].split("+")
                                        value = ""
                                        if len(value_) == 1 and value_[0].lower().startswith("resv(") and value_[0].endswith(")"):
                                            reserve = True
                                            length = parse_string(value_[0], resv=True)
                                        else:
                                            for vpart in value_:
                                                value += parse_string(vpart)
                                            length = int(len(value)/2)
                                        environment.variables[name] = (reserve, length, value)
                                    else:
                                        print("%s doesn't contain '%s'." % (filename, name))
                                        exit(1)
                            else:
                                for definition in list(filedata["PREPROCESSOR"].keys()):
                                    environment.macros[str(definition)] = "\n".join(filedata["PREPROCESSOR"][definition])
                                for variable in list(filedata["DATA"].keys()):
                                    length = 0
                                    reserve = False
                                    value_ = filedata["DATA"][variable].split("+")
                                    value = ""
                                    if len(value_) == 1 and value_[0].lower().startswith("resv(") and value_[0].endswith(")"):
                                        reserve = True
                                        length = parse_string(value_[0], resv=True)
                                    else:
                                        for vpart in value_:
                                            value += parse_string(vpart)
                                        length = int(len(value)/2)
                                    environment.variables[variable] = (reserve, length, value)
                        except json.JSONDecodeError:
                            print("Library '%s' is corrupted." % filename)
                            exit(1)
                ########

                elif line_.lower().startswith(".codesection"):
                    environment.custom_codesec_addr = True
                    if tryconvert(line_.split(" ")[1], int):
                        environment.codesec_addr = int(line_.split(" ")[1])
                    else:
                        print("Invalid code section address: '%s'." % line_.split(" ")[1])
                        exit(1)
                elif line_.lower().startswith(".datasection"):
                    environment.custom_datasec_addr = True
                    if tryconvert(line_.split(" ")[1], int):
                        environment.datasec_addr = int(line_.split(" ")[1])
                    else:
                        print("Invalid data section address: '%s'." % line_.split(" ")[1])
                        exit(1)
                elif line_.lower().startswith(".stacksize"):
                    if tryconvert(line_.split(" ")[1], int):
                        environment.stacksize = pack("I", int(line_.split(" ")[1]))
                    else:
                        print("Invalid stack size: '%s'." % line_.split(" ")[1])
                elif line_.lower().startswith(".debug"):
                    environment.options |= (1<<7)
                elif line_.lower().startswith(".nullstack"):
                    environment.options |= (1<<6)
                elif line_.lower().startswith(".dumpstack"):
                    environment.options |= (1<<5)
                else:
                    macroname = line_.split("=")[0].strip()
                    macrovalue = "=".join(l for l in line_.split("=")[1:]).strip()
                    environment.macros[macroname] = macrovalue
            elif remove_whitespaces(line_) == "}":
                environment.endpreprocessor = index_
                lines = content[index_+1:]
                macros = list(environment.macros.keys())
                macros.sort(key=lambda s: -len(s))
                for idx,ln in enumerate(lines):
                    for macro in macros:
                        if check_macro(ln, macro):
                            lines.pop(idx)
                            if len(environment.macros[macro].split("\n")) == 1: # one line macros
                                lines.insert(idx, ln.replace(macro, environment.macros[macro].split("\n")[0]))
                            else:
                                lines.insert(idx, ln.replace(macro, environment.macros[macro].split("\n")[0]))
                                [lines.insert(idx+1+index__, val) for index__, val in enumerate(environment.macros[macro].split("\n")[1:])]
                content[index_+1:] = lines
                break
    elif remove_whitespaces(line).lower() == "data{":
        for index_, line_ in enumerate(content[index+1:]):
            if line_ and not remove_whitespaces(line_) == "}":
                line_ = remove_whitespaces(line_)
                name = line_.split("=")[0]
                value_ = "=".join(vpart for vpart in line_.split("=")[1:])
                value_ = value_.split("+")
                value = ""
                reserve = False
                length = 0
                if len(value_) == 1 and value_[0].lower().startswith("resv(") and value_[0].endswith(")"):
                    reserve = True
                    length = parse_string(value_[0], resv=True)
                else:
                    for vpart in value_:
                        value += parse_string(vpart)
                    length = int(len(value)/2)
                environment.variables[name] = (reserve, length, value)
            elif remove_whitespaces(line_) == "}":
                environment.enddata = index_
                break
    elif remove_whitespaces(line).lower() == "code{":
        for index_, line_ in enumerate(content[index+1:]):
            if line_.lower().strip().split(" ")[0] in instructions.keys():
                parse_instruction(line_)
                environment.current_instruction += 1
            elif line_.lower().strip().endswith(":"):
                label_name = remove_whitespaces(line_)[:-1]
                environment.labels[label_name] = environment.current_instruction
            elif remove_whitespaces(line_) == "}":
                parse_instruction("eof")
                environment.endcode = index_
                break
            elif line_:
                print("\nUnknown instruction found: '%s'" % line_.strip())
                exit(1)

print("OK.")
if environment.verbose:
    print("Macros: {}\n\n".format(str(environment.macros)))
    print("Labels: {}\n\n".format(str(environment.labels)))
    print("Variables: {}\n\n".format(str(environment.variables)))
    print("Instructions: {}\n\n".format(str(environment.instructions)))

### COMPILING ###
#compiling program beginning so instructions like goto and invoke can calculate jump offset
bytecode = b"Oxy"+pack('B', environment.options)+environment.stacksize+pack("I", environment.datasec_addr if environment.custom_datasec_addr else 16)
pseudocomp = compile_variables(environment.variables)
instrs = []
stdout.write("Compiling instructions... ")

for iindex, instruction, arguments in environment.instructions:
    buf = instructions[instruction][1]
    if len(arguments) != len(instructions[instruction][2]):
        print("Too few/many arguments specified in instruction %d (%s). Arguments: %s" % (index+1, instruction, str(arguments)))
        exit(1)
    for index, argument in enumerate(instructions[instruction][2]):
        parsed = parse_string(arguments[index], code=True)
        possibilities = argument.split("/")
        for possibility in possibilities:
            if possibility == "short":
                if len(parsed) == 4: # parsed is in hexadecimal, thus *2
                    buf += unhexlify(parsed)
                    break
            elif possibility == "byte":
                if len(parsed) == 2:
                    buf += unhexlify(parsed)
                    break
            elif possibility == "variable":
                if arguments[index] in list(environment.variables.keys()):
                    buf += pack("I", list(environment.variables.keys()).index(arguments[index]))
                    break
            elif possibility == "instint" or possibility == "label":
                if len(parsed) == 8 or arguments[index] in list(environment.labels.keys()):
                    insint = None
                    if parsed in list(environment.labels.keys()):
                        insint = environment.labels[arguments[index]]
                    else:
                        insint = unpack("I", parsed.encode())[0]
                    length = len(bytecode+pseudocomp)+4
                    _insts = []
                    for inst in environment.instructions:
                        if inst[0] < insint:
                            _insts.append(inst)
                    length += calcsize(_insts)
                    buf += pack("I", length)
                    break
            elif possibility == "int":
                if len(parsed) == 8:
                    buf += unhexlify(parsed)
                    break
            elif possibility == "library":
                if arguments[index] in libraries.keys():
                    buf += libraries[arguments[index]]
                    break
            print("Wrong argument (%s) in instruction %d (%s)" % (arguments[index], iindex, instruction))
            exit(1)
    instrs.append(buf)

print("OK.")
stdout.write("updating code section address... ")
bytecode += pack("I", (environment.codesec_addr if environment.custom_codesec_addr else 16+len(pseudocomp)))
print("OK.")
stdout.write("adding data and code to output... ")
bytecode += pseudocomp
bytecode += b"".join(inst for inst in instrs)
print("OK.")

if environment.verbose:
    print("Custom Codesec Addr:",environment.custom_codesec_addr)
    print("Custom Datasec Addr:",environment.custom_datasec_addr)
    header = bytecode[0:3]
    data = pseudocomp
    dbgvars = ""
    for i in range(len(environment.variables.keys())):
        length, data = get_dbglen(data)
        value, data = get_dbgstr(data, length)
        dbgvars += "{} : {}\n".format(length, hexlify(value).decode())
    print("Result:\nHeader: {}\nOptions: {}\n\n----Data Section----\n{}".format(hexlify(header).decode(), bin(bytecode[3]), dbgvars))

with open(environment.output_filename, "wb") as outfile:
    outfile.write(bytecode)
time_end = time()
print("Finished compiling in %.15f seconds." % (time_end-time_start))
