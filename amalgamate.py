

OUTPUT = 'audit.h'


LIB_DIR = 'audit'
STARTER  =f'main.c'


def generate_amalgamation(file:str)->str:

    with open(f'{LIB_DIR}/{file}','r') as arq:
        code = arq.read()


    generated = ''
    lines = code.split('\n')
    current_line = 0

    for line in lines:
        current_line += 1
        #Testing if is an project include
        

        if line.startswith('#include'):
            try:
                path = line.split(' ')[1]
            except IndexError:
                print(f'Error on include on line {current_line}')
                exit(1)

            if '"' in path:
                path = path.replace('"','')
                try:
                    with open(f'{LIB_DIR}/{path}','r') as arq:
                        generated += arq.read() + '\n'
                        continue      
                except FileNotFoundError:
                    print(f'File {path} not found on line {current_line}')
                    exit(1)
        else:
            generated += line + '\n'
    return generated


code  = generate_amalgamation(STARTER)

with open(OUTPUT,'w') as arq:
    arq.write(code)


        
        