import cpymem

a=19
b=20
print(a,b)
cpymem.PingPong(a,b)
print(a,b)


def test():
    c=1
    d=5
    print(c,d)
    cpymem.PingPong(c,d)
    print(c,d)

test()

