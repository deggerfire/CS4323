# Useing sympy for this, because birds arnt real
from sympy import *
import re

DEBUG = False
# Genrates the symbols for 1 line of code
def Symbol_Gen(line):
    # Clear all the white space
    line = re.sub("\s", "", line)
    if DEBUG: print(line) # Test print so you can get indexs
    # For a dst[a] = src[b] line
    if(line[6] == '='):
        line = re.sub("\D", "", line)
        return [symbols(line[0] + '=' + line[1])]

# Check if code is symbolically equal to correct
def Check_Symbolically(correct, code):
    # Make the code symbol list
    codeSymList = []
    for line in code:
        # Use Symbol_Gen to get the symbols on the line
        for sym in Symbol_Gen(line):
            codeSymList.append(sym)

    if DEBUG: print(codeSymList) # Use this to see what symbols you made
            
    # Go though the codes symbol list
    for line in codeSymList:
        # Check if it is in the correct symbols, if so remove it
        if line in correct:
            correct.remove(line)
        # Else they are not symbolically equal and we are done
        else:
            print("Not Symbolically Equal")
            return False

    # Check is correct is empty (cause everything should of been removed)
    if len(correct) == 0:
        # If it is they are equal
        print("Symbolically Equal")
        return True
    else:
        # Else they are not equal
        print("Not Symbolically Equal")
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

# Read in the code, handwaving it for now
code_in = ["dst[2] = src[3];",
        "dst[0] = src[1];",
        "dst[1] = src[2];",
        "dst[3] = src[0];",
        "dst[4] = src[4];",
        "dst[5] = src[5];",
        "dst[6] = src[6];",
        "dst[7] = src[7];",]

print(Check_Symbolically(correct_in, code_in))