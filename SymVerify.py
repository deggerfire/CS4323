# Useing sympy for this, because birds arnt real
from sympy import *
import re

###########################################
##     Test object based on real ones    ##
###########################################
KIND_MEM_TO_REG = 1
KIND_REG_TO_REG = 2
KIND_REG_TO_MEM = 3

I_ARR = 1
O_ARR = 2
IO_REG = 3

class Instruction:
    def __init__(self, kind, io_kind) -> None:
        self.kind = kind
        self.io_kind = io_kind

    def code() -> str:
        raise Exception("Illegal instruction")
    
    def Symbol_Gen(self) -> str:
        raise Exception("Illegal Symbol")

class Load(Instruction):
    def __init__(self, src_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_MEM_TO_REG, I_ARR)

        self.src_arr = src_arr
        self.offset = offset


    def dst_name(self) -> str:
        return f'{self.src_arr}_{self.offset}'


    def code(self) -> str:
        assert ("" != self.src_arr)

        return f'{DATA_TYPE} {self.dst_name()} = {self.src_arr}[{self.offset}];'

    def Symbol_Gen(self, codeSymList) -> str:
        codeSymList.append(f'{self.dst_name()}={self.offset}')
        print("Load: " + str(codeSymList))

class Store(Instruction):
    def __init__(self, src: str = "", dst_arr: str = "", offset: int = 0) -> None:
        super().__init__(KIND_REG_TO_MEM, O_ARR)

        self.src = src
        self.dst_arr = dst_arr
        self.offset = offset


    def src_name(self) -> str:
        return self.src


    def code(self) -> str:
        assert ("" != self.dst_arr)
        assert ("" != self.src)

        return f'{DATA_TYPE} {self.dst_arr}[{self.offset}] = {self.src};'

    def Symbol_Gen(self, codeSymList) -> str:
        for symbol in codeSymList:
            if f'{self.src}' in symbol.split("="):
                codeSymList.append(symbols(f'{str(symbol).split("=")[1]}={self.offset}'))
                codeSymList.remove(symbol)
        print("Store: " + str(codeSymList))
    
############################
##The code being worked on##
############################
DEBUG = True
# Check if code is symbolically equal to correct
def Check_Symbolically(correct, instructions):
    # Make the code symbol list
    codeSymList = []
    for instr in instructions:
        # Use Symbol_Gen method to get the symbols from instruction
        instr.Symbol_Gen(codeSymList)

    if DEBUG: print(codeSymList) # Use this to see what symbols you made
            
    # Go though the codes symbol list
    for line in codeSymList:
        # Check if it is in the correct symbols, if so remove it
        if line in correct:
            correct.remove(line)
        # Else they are not symbolically equal and we are done
        else:
            if DEBUG: print("Not Symbolically Equal")
            return False

    # Check is correct is empty (cause everything should of been removed)
    if len(correct) == 0:
        # If it is they are equal
        if DEBUG: print("Symbolically Equal")
        return True
    else:
        # Else they are not equal
        if DEBUG: print("Not Symbolically Equal")
        return False

########################
##     Test inputs    ##
########################
# Set what the correct list of symbols is
correct_in = list(symbols(
                '0=1 '
                '1=2 '
                '2=3 '
                '3=0 '
                '4=4 '
                '5=5 '
                '6=6 '
                '7=7 '))

# Make test instructions
intr = [Load("src", 0), Store("src_0", "dst", 1)]

print(Check_Symbolically(correct_in, intr))
print(correct_in)