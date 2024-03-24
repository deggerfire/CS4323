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

# Test for no return
class Nop(Instruction):
    def __init__(self, comment) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.comment = comment


    def code(self) -> str:
        return f'// nop: {self.comment}'
    
    def Symbol_Gen(self) -> str:
        return []

# Test for many return
class Nop2(Instruction):
    def __init__(self, comment) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.comment = comment

    def code(self) -> str:
        return f'// nop: {self.comment}'
    
    def Symbol_Gen(self) -> str:
        return list(symbols('0=1 '
                '1=2 '
                '2=3 '
                '3=0 '
                '4=4 '
                '5=5 '
                '6=6 '))
    
# Test for 1 return
class Nop3(Instruction):
    def __init__(self, comment) -> None:
        super().__init__(KIND_REG_TO_REG, IO_REG)

        self.comment = comment


    def code(self) -> str:
        return f'// nop: {self.comment}'
    
    def Symbol_Gen(self) -> str:
        return [symbols('7=7')]
    
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
        for sym in instr.Symbol_Gen():
            codeSymList.append(sym)

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
intr = [Nop("test"), Nop2("test"), Nop3("test")]

print(Check_Symbolically(correct_in, intr))