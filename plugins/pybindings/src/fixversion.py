# pragmatically fix version number
import sys
verin=sys.argv[1]
verout= verin[0:-1] + "." + str(ord(verin[-1])-ord('a'))
print(verout)
