import math

if __name__ == '__main__':
    f = lambda x: math.sin((x - 120) / 4) * 40 + 40

    with open("velocimeter.simulacrum", "w") as file:
        for i in range(0, 100):
            input_value = (f(i))
            print(input_value)
            file.write(str(input_value)+'\n')
