int g(int x)
{
    return x + 3;
}

int f(int x)
{
    int i = 11;
    return g(x) + i;
}

int main(void)
{
    return f(8) + 1;
}
