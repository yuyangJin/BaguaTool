
asemble_type_name = ["LOAD", "STORE", "ADD",
                     "SUB", "MUL", "IMUL", "DIV", "IDIV", "others"]
# example ['loop-1.1', [4,0,1,0,2,2,1,0]]


class asembleInstructionData(object):
    def __init__(self, asm_file, funcs):
        self.asm_file = asm_file
        self.funcs = funcs
        #self.data = [[] for i in range(len(self.funcs))]
        self.data = {}
        self.readAsembleInstructionDataFile()

    def readAsembleInstructionDataFile(self):
        #self.data = [[] for i in range(len(self.funcs))]
        with open(self.asm_file) as f1:
            asm_lines = f1.readlines()
        f1.close()

        func = ""
        for i in range(len(asm_lines)):
            asm_line = asm_lines[i]
            asm_line_split = asm_line.strip().split(' ')
            if asm_line_split[0] == 'L':
                i += 1
                asm_line = asm_lines[i]
                types = [int, int, int, int, int, int, int, int, int]
                args = list(map(lambda x: x[0](x[1]), zip(
                                types, asm_line.strip().split(' '))))
                #self.data[func_i].append([asm_line_split[1], args])
                self.data[func][asm_line_split[1]] = args
            elif asm_line_split[0] == 'F':
                #self.data[func_i].append(asm_line_split[1])
                func = asm_line_split[1]
                self.data[func] = {}

        #print(self.data)

        print("read self.asm_file")

    