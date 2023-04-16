import sys
templete_quartus='''
WIDTH={};
DEPTH={};

ADDRESS_RADIX=UNS;
DATA_RADIX=UNS;

CONTENT BEGIN
{}
END;
'''

templete_vivado='''
memory_initialization_radix = 10;
memory_initialization_vector =
{}

'''

templete_gowin='''
#File_format=Hex
#Address_depth=10000
#Data_width=32
{}

'''

data=''
addr=0;
#print( sys.argv[1])
for i in sys.argv[1].split('\n'):
 #print("{:032b}".format(i))
    data+="{}  : {:d};\n".format(addr,int(i,16))
    addr+=1

print('///////////////////////////// quartus version memory file')
print(templete_quartus.format(32,addr,data))


data = ''
for i in sys.argv[1].split('\n'):
 #print("{:032b}".format(i))
    data+="{:d},\n".format(int(i,16))


print('///////////////////////////// vivado version memory file')
print(templete_vivado.format(data.rstrip(',\n')))

print('///////////////////////////// gowin version hex memory file')

data = ''
for i in sys.argv[1].split('\n'):
 #print("{:032b}".format(i))
    data+="{}\n".format(i)
print(templete_gowin.format(data.rstrip(',\n')))
f =open('rom','w')
f.write(templete_gowin.format(data.rstrip(',\n')))
f.close()

print('///////////////////////////// binary represenation')
j=0
k=0
for i in sys.argv[1].split('\n'):
  
  line="{:032b}".format(int(i,16))
  print(str(j) +": "+str(k)+" : "+line[:25]+" "+line[25:]+"        hex:"+str(i))
  j =j+4
  k=k+1

#risc-v ops to opcodes


print('///////////////////////////// opcode represenation')
