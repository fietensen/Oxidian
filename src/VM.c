#include <Oxidian/VM.h>

// the main stuff

int init_oxy(char *filename, int argc, char **argv, char **envp) {
#if OPENSSL
#ifdef _WIN32
    printf("[NOTE] OpenSSL cannot be used on Windows. Please wait until the release of SSL support for windows.\n");
    return 0;
#endif
#endif

    // fgets will include a newline character
    // into the filename so we must remove it
    for(int i=0;i<256;i++){if(filename[i]=='\n'){break;} else if(filename[i]==0){break;}};

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] An error occoured while trying to open the specified file.\n");
        fprintf(stderr, "[Errno. %d]: %s\n", errno, strerror(errno));
        return 1;
    }

    // Setup environment
    OEnvironment Oenviron;
    setup_environment(&Oenviron);
    Oenviron.fp = fp;

    // Parse File Magic
    fread(Oenviron.magic_number, 3, 1, fp);
    if (strncmp(Oenviron.magic_number, "Oxy", 3)) {
        printf("%s\n", Oenviron.magic_number);
        fprintf(stderr, "[ERROR] Invalid file format.\n");
        return 1;
    }

    Oenviron.program_setting = fgetc(fp);
    Oenviron.debug = Oenviron.program_setting>>7&1;
    //Oenviron.debug = true;
    Oenviron.nullstack = (bool)Oenviron.program_setting>>6&1;
    Oenviron.dumpstack = (bool)Oenviron.program_setting>>5&1;

    fread(Oenviron.temp_addr, 4, 1, fp);
    Oenviron.stack_size = C2UI(Oenviron.temp_addr);
    if (Oenviron.debug) printf("[DEBUG] Stack size is 0x%x (%u)\n", Oenviron.stack_size, Oenviron.stack_size);

    Oenviron.stack = malloc(Oenviron.stack_size);

    if (Oenviron.stack == NULL) {
        fprintf(stderr, "[ERROR] Unable to allocate space for the stack. Free up memory or decrease stack size.\n");
        return 1;
    }

    if (Oenviron.nullstack) {
        if (Oenviron.debug) printf("[DEBUG] Nulling stack (0x%x (%u) bytes)\n", Oenviron.stack_size, Oenviron.stack_size);
        memset(Oenviron.stack, 0, Oenviron.stack_size);
    }

    fread(Oenviron.temp_addr, 4, 1, fp);
    Oenviron.data_section = C2UI(Oenviron.temp_addr);
    if (Oenviron.debug) printf("[DEBUG] Data section is at 0x%x (%u)\n", Oenviron.data_section, Oenviron.data_section);

    fread(Oenviron.temp_addr, 4, 1, fp);
    Oenviron.code_section = C2UI(Oenviron.temp_addr);
    if (Oenviron.debug) printf("[DEBUG] Code section is at 0x%x (%u)\n", Oenviron.code_section, Oenviron.code_section);


    // getting the total length of the variable data / number of variables
    fseek(fp, Oenviron.data_section, SEEK_SET);
    Oenviron.program_pointer = Oenviron.data_section;
    bool resv = false;

    while (Oenviron.program_pointer < Oenviron.code_section) {
        fread(Oenviron.temp_addr, 4, 1, fp);
        Oenviron.buf = C2UI(Oenviron.temp_addr);
        fread(&resv, sizeof(bool), 1, fp);

        Oenviron.program_pointer += (resv ? 0 : Oenviron.buf) + 5;

        Oenviron.data_length += Oenviron.buf;
        Oenviron.data_elements++;
        if (Oenviron.debug) {
            printf("[DEBUG] hex: %4x :: current_position: %ld\n", Oenviron.buf, ftell(fp));
            printf("[DEBUG] Length of Element %d = %u\n", Oenviron.data_elements, Oenviron.buf);
        }
        fseek(fp, Oenviron.program_pointer, SEEK_SET);
    }

    Oenviron.variables = malloc(sizeof(OVariable)*Oenviron.data_elements);
    //Oenviron.element_sizes = (unsigned int*)malloc(sizeof(unsigned int)*Oenviron.data_elements);

    if (Oenviron.debug) {
        printf("[DEBUG] Data Elements: %d\n", Oenviron.data_elements);
        printf("[DEBUG] Total datasize: %u\n", Oenviron.data_length);
    }

    // reading individual variable lengths
    fseek(fp, Oenviron.data_section, SEEK_SET);
    Oenviron.program_pointer = Oenviron.data_section;
    Oenviron.data_elements = 0;

    while (Oenviron.program_pointer < Oenviron.code_section) {
        fread(Oenviron.temp_addr, 4, 1, fp);
        Oenviron.buf = C2UI(Oenviron.temp_addr);
        Oenviron.variables[Oenviron.data_elements].size = Oenviron.buf;
        //Oenviron.element_sizes[Oenviron.data_elements] = Oenviron.buf;
        Oenviron.data_elements++;
        Oenviron.program_pointer += Oenviron.buf + 5;
        fseek(fp, Oenviron.program_pointer, SEEK_SET);
    }

    // finally read in all the variable values
    fseek(fp, Oenviron.data_section, SEEK_SET);

    for (unsigned int i=0;i<Oenviron.data_elements;i++) {
        Oenviron.variables[i].value = malloc(Oenviron.variables[i].size+1);
        memset(Oenviron.variables[i].value, 0, Oenviron.variables[i].size+1);
        readint(fp);
        fread(&resv, sizeof(bool), 1, fp);
        if (!resv) fread(Oenviron.variables[i].value, Oenviron.variables[i].size, 1, fp);
        if (Oenviron.debug && !resv) printf("[DEBUG] variables[%u] = %s\n", i, (char*)Oenviron.variables[i].value);
        else if (Oenviron.debug) printf("[DEBUG] variables[%u] = RESV(%u)\n", i, Oenviron.variables[i].size+1);
    }

#if NETLIB
    init_networking();
#endif

    // Now the parsing stuff
    Oenviron.program_pointer = Oenviron.code_section;
    fseek(fp, Oenviron.code_section, SEEK_SET);

    if (Oenviron.debug) printf("-=====CODE======-\n");
    
    bool quit = false;

    while (!quit) {
        Oenviron.instructions_length++;
        fread(Oenviron.temp_short, 2, 1, fp);
        Oenviron.current_instruction = C2US(Oenviron.temp_short);
        if (Oenviron.debug) printf("[DEBUG] Instruction: %hu; Offset: %u\n", Oenviron.current_instruction, Oenviron.program_pointer);
        Oenviron.program_pointer += 2;
        if (!Oenviron.iscase) {
            switch(Oenviron.current_instruction) {
                case INST_LOADLIB:
                    fread(Oenviron.temp_short, 2, 1, fp);
                    Oenviron.current_library = C2US(Oenviron.temp_short);
                    Oenviron.program_pointer += INST_LOADLIB_S;
                    if (Oenviron.debug) printf("[DEBUG] LOADLIB(%hu)\n", Oenviron.current_library);
                    break;
                case INST_SETARG:
                    Oenviron.argument_number = fgetc(fp);
                    fread(Oenviron.temp_addr, 4, 1, fp);
                    Oenviron.data_number = C2UI(Oenviron.temp_addr);
                    Oenviron.arguments[Oenviron.argument_number] = &Oenviron.variables[Oenviron.data_number];
                    Oenviron.program_pointer += INST_SETARG_S;
                    if (Oenviron.debug) printf("[DEBUG] SETARG(%d, VARIABLES[%u])\n", Oenviron.argument_number, Oenviron.data_number);
                    break;
                case INST_CALL:
                    fread(Oenviron.temp_short, 2, 1, fp);
                    Oenviron.call_number = C2US(Oenviron.temp_short);
                    if (Oenviron.debug) printf("[DEBUG] CALL(%hu)\n", Oenviron.call_number);
                    if (handle_call(&Oenviron)) return 1;
                    Oenviron.program_pointer += INST_CALL_S;
                    break;
                case INST_GOTO:
                    fread(Oenviron.temp_addr, 4, 1, fp);
                    fseek(fp, C2UI(Oenviron.temp_addr), SEEK_SET);
                    Oenviron.program_pointer = C2UI(Oenviron.temp_addr);
                    if (Oenviron.debug) printf("[DEBUG] GOTO(%u)\n", Oenviron.program_pointer);
                    break;
                case INST_ADD:
                    Oenviron.temp_i[0] = readint(fp); // data_item index
                    Oenviron.temp_i[1] = readint(fp); // Value to add
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[3] = Oenviron.temp_i[1] + Oenviron.temp_i[2];
                    int_to_char(Oenviron.temp_i[3], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_ADD_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEM[%u] = %u + %u\n", Oenviron.temp_i[0], Oenviron.temp_i[1], Oenviron.temp_i[2]);
                    break;
                case INST_SUB:
                    Oenviron.temp_i[0] = readint(fp); // data_item index
                    Oenviron.temp_i[1] = readint(fp); // Value to substract
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[3] = Oenviron.temp_i[1] - Oenviron.temp_i[2];
                    int_to_char(Oenviron.temp_i[3], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_SUB_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEM[%u] = %u - %u\n", Oenviron.temp_i[0], Oenviron.temp_i[1], Oenviron.temp_i[2]);
                    break;
                case INST_INC:
                    Oenviron.temp_i[0] = readint(fp); // data_item index
                    Oenviron.temp_i[1] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[1]++;
                    int_to_char(Oenviron.temp_i[1], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_INC_S;
                    if (Oenviron.debug) printf("[DEBUG] INC(ITEM[%u])\n", Oenviron.temp_i[0]);
                    break;
                case INST_DEC:
                    Oenviron.temp_i[0] = readint(fp); // data_item index
                    Oenviron.temp_i[1] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[1]--;
                    int_to_char(Oenviron.temp_i[1], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_INC_S;
                    if (Oenviron.debug) printf("[DEBUG] DEC(ITEM[%u])\n", Oenviron.temp_i[0]);
                    break;
                case INST_CMP:
                    Oenviron.temp_i[0] = readint(fp); // data_item index
                    Oenviron.temp_i[1] = readint(fp); // value to cmp
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    if (Oenviron.temp_i[2] > Oenviron.temp_i[1]) Oenviron.compare_result = 1;
                    else if (Oenviron.temp_i[2] < Oenviron.temp_i[1]) Oenviron.compare_result = -1;
                    else Oenviron.compare_result = 0;
                    Oenviron.program_pointer += INST_CMP_S;
                    if (Oenviron.debug) printf("[DEBUG] CMP(%d, %d) => %d", Oenviron.temp_i[2], Oenviron.temp_i[1], Oenviron.compare_result);
                    break;
                case INST_CL:
                    if (Oenviron.compare_result!=-1) Oenviron.iscase=true;
                    if (Oenviron.debug) printf("[DEBUG] COMPARE_LOWER => %s", (Oenviron.compare_result==-1) ? "true" : "false");
                    break;
                case INST_CH:
                    if (Oenviron.compare_result!=1) Oenviron.iscase=true;
                    if (Oenviron.debug) printf("[DEBUG] COMPARE_HIGHER => %s", (Oenviron.compare_result==1) ? "true" : "false");
                    break;
                case INST_CE:
                    if (Oenviron.compare_result!=0) Oenviron.iscase=true;
                    if (Oenviron.debug) printf("[DEBUG] COMPARE_EQUAL => %s", (Oenviron.compare_result==0) ? "true" : "false");
                    break;
                case INST_SSPP:
                    Oenviron.stackpointerposition = readint(fp);
                    Oenviron.program_setting += INST_SSPP_S;
                    if (Oenviron.debug) printf("[DEBUG] Stack Pointer = %u\n", Oenviron.stackpointerposition);
                    break;
                case INST_iPSH:
                    Oenviron.temp_i[0] = readint(fp);
                    strcpy((char*)&Oenviron.stack[Oenviron.stackpointerposition], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.stackpointerposition += strlen(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iPSH_S;
                    if (Oenviron.debug) printf("[DEBUG] PUSH(ITEM[%u])\n", Oenviron.temp_i[0]);
                    break;
                case INST_vPSH:
                    fread(&Oenviron.stack[Oenviron.stackpointerposition], 2, 1, fp);
                    Oenviron.stackpointerposition += 2;
                    Oenviron.program_pointer += INST_vPSH_S;
                    if (Oenviron.debug) printf("[DEBUG] PUSH(%x%x)\n", Oenviron.stack[Oenviron.stackpointerposition-2], Oenviron.stack[Oenviron.stackpointerposition-1]);
                    break;
                case INST_POP:
                    Oenviron.temp_i[0] = readint(fp); // item the data should be popped in
                    Oenviron.temp_i[1] = readint(fp); // pop size
                    for (unsigned int i=0;i<(unsigned int)Oenviron.temp_i[1];i++)
                        ((char*)Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value)[i] = Oenviron.stack[--Oenviron.stackpointerposition];
                    Oenviron.program_pointer += INST_POP_S;
                    if (Oenviron.debug) printf("[DEBUG] POP(ITEM[%u], SIZE=%u)\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1]);
                    break;
                case INST_iPOP:
                    Oenviron.temp_i[0] = readint(fp); // item the data should be popped in
                    Oenviron.temp_i[1] = readint(fp); // item containing pop size
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value); // get temp_i[1] as integer
                    for (unsigned int i=0;i<(unsigned int)Oenviron.temp_i[2];i++)
                        ((char*)Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value)[i] = Oenviron.stack[--Oenviron.stackpointerposition];
                    Oenviron.program_pointer += INST_iPOP_S;
                    if (Oenviron.debug) printf("[DEBUG] POP(ITEM[%u], SIZE=ITEM[%u]=%u)\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1], (unsigned int)Oenviron.temp_i[2]);
                    break;
                case INST_iGETL:
                    Oenviron.temp_i[0] = readint(fp);
                    Oenviron.temp_i[1] = readint(fp);
                    int_to_char(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].size, Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iGETL_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEM[%u] = ITEM[%u].length\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1]);
                    break;
                case INST_iSSPP:
                    Oenviron.temp_i[0] = readint(fp);
                    Oenviron.stackpointerposition = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iSSPP_S;
                    if (Oenviron.debug) printf("[DEBUG] Stack Pointer = ITEM[%u]\n", (unsigned int)Oenviron.temp_i[0]);
                    break;
                case INST_iADD:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // index of item to add
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value); // convert item to integer
                    Oenviron.temp_i[3] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value); // convert item to add to integer
                    Oenviron.temp_i[4] = (unsigned int)Oenviron.temp_i[2]+(unsigned int)Oenviron.temp_i[3];
                    int_to_char((unsigned int)Oenviron.temp_i[4], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iADD_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEM[%u] = %u + %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[2], (unsigned int)Oenviron.temp_i[3]);
                    break;
                case INST_iSUB:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // index of item to subtract
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value); // convert item to integer
                    Oenviron.temp_i[3] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value); // convert item to subtract to integer
                    Oenviron.temp_i[4] = (unsigned int)Oenviron.temp_i[2]-(unsigned int)Oenviron.temp_i[3];
                    int_to_char((unsigned int)Oenviron.temp_i[4], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iSUB_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEM[%u] = %u - %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[2], (unsigned int)Oenviron.temp_i[3]);
                    break;
                case INST_iCMP:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // item index to cmp
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value); // item to cmp -> int
                    Oenviron.temp_i[3] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value); // item -> int
                    if (Oenviron.temp_i[3] > Oenviron.temp_i[2]) Oenviron.compare_result = 1;
                    else if (Oenviron.temp_i[3] < Oenviron.temp_i[2]) Oenviron.compare_result = -1;
                    else Oenviron.compare_result = 0;
                    Oenviron.program_pointer += INST_iCMP_S;
                    if (Oenviron.debug) printf("[DEBUG] CMP(%d, %d) => %d\n", Oenviron.temp_i[3], Oenviron.temp_i[2], Oenviron.compare_result);
                    break;
                case INST_AND:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // value to AND with
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    int_to_char((unsigned int)Oenviron.temp_i[1] & (unsigned int)Oenviron.temp_i[2], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_AND_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = %u & %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1], (unsigned int)Oenviron.temp_i[2]);
                    break;
                case INST_iAND:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // index of item to AND
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[3] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value);
                    int_to_char((unsigned int)Oenviron.temp_i[2] & (unsigned int)Oenviron.temp_i[3], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iAND_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = %u & %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[2], (unsigned int)Oenviron.temp_i[3]);
                    break;
                case INST_OR:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // value to OR with
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    int_to_char((unsigned int)Oenviron.temp_i[1] | (unsigned int)Oenviron.temp_i[2], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_AND_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = %u | %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1], (unsigned int)Oenviron.temp_i[2]);
                    break;
                case INST_iOR:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // index of item to OR
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[3] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value);
                    int_to_char((unsigned int)Oenviron.temp_i[2] | (unsigned int)Oenviron.temp_i[3], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iAND_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = %u | %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[2], (unsigned int)Oenviron.temp_i[3]);
                    break;
                case INST_XOR:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // value to XOR with
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    int_to_char((unsigned int)Oenviron.temp_i[1] ^ (unsigned int)Oenviron.temp_i[2], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_AND_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = %u ^ %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1], (unsigned int)Oenviron.temp_i[2]);
                    break;
                case INST_iXOR:
                    Oenviron.temp_i[0] = readint(fp); // item index
                    Oenviron.temp_i[1] = readint(fp); // index of item to XOR
                    Oenviron.temp_i[2] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.temp_i[3] = C2UI(Oenviron.variables[(unsigned int)Oenviron.temp_i[1]].value);
                    int_to_char((unsigned int)Oenviron.temp_i[2] ^ (unsigned int)Oenviron.temp_i[3], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iAND_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = %u ^ %u\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[2], (unsigned int)Oenviron.temp_i[3]);
                    break;
                case INST_iREF:
                    Oenviron.temp_i[0] = readint(fp); // item where we want to store reference to item2
                    Oenviron.temp_i[1] = readint(fp); // reference to item2
                    int_to_char((unsigned int)Oenviron.temp_i[1], Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_iREF_S;
                    if (Oenviron.debug) printf("[DEBUG] ITEMS[%u] = reference(ITEM[%u])\n", (unsigned int)Oenviron.temp_i[0], (unsigned int)Oenviron.temp_i[1]);
                    break;
                case INST_INVOKE:
                    Oenviron.temp_i[0] = readint(fp); // Get address of routine
                    fseek(fp, (unsigned int)Oenviron.temp_i[0], SEEK_SET);
                    Oenviron.function_depth++;
                    Oenviron.return_addresses[Oenviron.function_depth] = Oenviron.program_pointer-2; // Store return address
                    Oenviron.program_pointer = (unsigned int)Oenviron.temp_i[0];
                    if (Oenviron.debug) printf("[DEBUG] Invoking routine at %u\n", Oenviron.program_pointer);
                    break;
                case INST_RETURN:
                    // little explanation here: instruction is short, aka 2 bytes. invoke address is 4 bytes, stored int INST_INVOKE_S 
                    fseek(fp, Oenviron.return_addresses[Oenviron.function_depth]+INST_INVOKE_S+2, SEEK_SET); // return to address that invoked (skip invoke to prevent recursion)
                    Oenviron.program_pointer = Oenviron.return_addresses[Oenviron.function_depth--]+INST_INVOKE_S+2;
                    if (Oenviron.debug) printf("[DEBUG] Returning from routine invoked at %u. New position: %u\n", Oenviron.return_addresses[Oenviron.function_depth+1], Oenviron.return_addresses[Oenviron.function_depth+1]+INST_INVOKE_S+2);
                    break;
                case INST_GSPP:
                    Oenviron.temp_i[0] = readint(fp);
                    int_to_char(Oenviron.stackpointerposition, Oenviron.variables[(unsigned int)Oenviron.temp_i[0]].value);
                    Oenviron.program_pointer += INST_GSPP_S;
                    if (Oenviron.debug) printf("[DEBUG] Stored Stack Pointer in ITEMS[%u]\n", (unsigned int)Oenviron.temp_i[0]);
                    break;
                case INST_CNE:
                    if (Oenviron.compare_result==0)Oenviron.iscase=true;
                    if (Oenviron.debug) printf("[DEBUG] COMPARE_NOT_EQUAL => %s\n", (Oenviron.compare_result!=0) ? "true" : "false");
                    break;
                case INST_CNH:
                    if (Oenviron.compare_result==1)Oenviron.iscase=true;
                    if (Oenviron.debug) printf("[DEBUG] COMPARE_NOT_HIGHER => %s\n", (Oenviron.compare_result!=1) ? "true" : "false");
                    break;
                case INST_CNL:
                    if (Oenviron.compare_result==-1)Oenviron.iscase=true;
                    if (Oenviron.debug) printf("[DEBUG] COMPARE_NOT_LOWER => %s\n", (Oenviron.compare_result!=-1) ? "true" : "false");
                    break;
                case INST_CHKERR:
                    Oenviron.temp_i[0] = readint(fp);
                    if (Oenviron.error_number != (unsigned int)Oenviron.temp_i[0]) { Oenviron.iscase = true; };
                    if (Oenviron.debug) printf("[DEBUG] COMPARE ERROR (%u) and (%u)\n", Oenviron.error_number, Oenviron.temp_i[0]);
                    Oenviron.program_pointer += INST_CHKERR_S;
                    break;
                case INST_EOF:
                    quit=true;
                    break;
                default:
                    if (Oenviron.current_instruction != 0xffff) {
                        if (runtimeerror(ERR_NOINST, &Oenviron)) {
                            clean_environment(&Oenviron);
                            fclose(fp);
                            return 1;
                        }
                    }
                    break;
            }
            Oenviron.instruction_before = Oenviron.current_instruction;
        } else {
            switch (Oenviron.current_instruction) {
                case INST_LOADLIB:Oenviron.program_pointer+=INST_LOADLIB_S;break;
                case INST_SETARG:Oenviron.program_pointer+=INST_SETARG_S;break;
                case INST_CALL:Oenviron.program_pointer+=INST_CALL_S;break;
                case INST_GOTO:Oenviron.program_pointer+=INST_GOTO_S;break;
                case INST_ADD:Oenviron.program_pointer+=INST_ADD_S;break;
                case INST_SUB:Oenviron.program_pointer+=INST_SUB_S;break;
                case INST_CMP:Oenviron.program_pointer+=INST_CMP_S;break;
                case INST_CL:Oenviron.program_pointer+=INST_CL_S;break;
                case INST_CH:Oenviron.program_pointer+=INST_CH_S;break;
                case INST_CE:Oenviron.program_pointer+=INST_CE_S;break;
                case INST_INC:Oenviron.program_pointer+=INST_INC_S;break;
                case INST_DEC:Oenviron.program_pointer+=INST_DEC_S;break;
                case INST_SSPP:Oenviron.program_pointer+=INST_SSPP_S;break;
                case INST_iPSH:Oenviron.program_pointer+=INST_iPSH_S;break;
                case INST_vPSH:Oenviron.program_pointer+=INST_vPSH_S;break;
                case INST_iPOP:Oenviron.program_pointer+=INST_iPOP_S;break;
                case INST_iGETL:Oenviron.program_pointer+=INST_iGETL_S;break;
                case INST_iSSPP:Oenviron.program_pointer+=INST_iSSPP_S;break;
                case INST_iADD:Oenviron.program_pointer+=INST_iADD_S;break;
                case INST_iSUB:Oenviron.program_pointer+=INST_iSUB_S;break;
                case INST_iCMP:Oenviron.program_pointer+=INST_iCMP_S;break;
                case INST_AND:Oenviron.program_pointer+=INST_AND_S;break;
                case INST_iAND:Oenviron.program_pointer+=INST_iAND_S;break;
                case INST_OR:Oenviron.program_pointer+=INST_OR_S;break;
                case INST_iOR:Oenviron.program_pointer+=INST_iOR_S;break;
                case INST_XOR:Oenviron.program_pointer+=INST_XOR_S;break;
                case INST_iXOR:Oenviron.program_pointer+=INST_iXOR_S;break;
                case INST_iREF:Oenviron.program_pointer+=INST_iREF_S;break;
                case INST_POP:Oenviron.program_pointer+=INST_POP_S;break;
                case INST_INVOKE:Oenviron.program_pointer+=INST_INVOKE_S;break;
                case INST_RETURN:Oenviron.program_pointer+=INST_RETURN_S;break;
                case INST_GSPP:Oenviron.program_pointer+=INST_GSPP_S;break;
                case INST_CNE:Oenviron.program_pointer+=INST_CNE_S;break;
                case INST_CNH:Oenviron.program_pointer+=INST_CNH_S;break;
                case INST_CNL:Oenviron.program_pointer+=INST_CNL_S;break;
                case INST_CHKERR:Oenviron.program_pointer+=INST_CHKERR_S;break;
                case INST_EOF:Oenviron.program_pointer+=INST_EOF_S;break;
            }
            if (Oenviron.debug) printf("[DEBUG] Skipped one statement, new Position: %u\n", Oenviron.program_pointer);
            fseek(fp, Oenviron.program_pointer, SEEK_SET);
            Oenviron.iscase = false;
        }
    }

#if NETLIB
    clean_networking();
#endif

    if (Oenviron.dumpstack) {
        printf("[DEBUG] Dumping the stack to \"stack.bin\"...\n");
        FILE* stackdmp = fopen("stack.bin", "w");
        fwrite(Oenviron.stack, Oenviron.stack_size, 1, stackdmp);
        fclose(stackdmp);
    }

    clean_environment(&Oenviron);
    fclose(fp);

    if (Oenviron.debug) printf("[DEBUG] File execution finished.\n");
    return 0;
}
