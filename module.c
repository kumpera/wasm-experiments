#include <stdio.h>

//FIXME all of those features 
extern float read_feature(const char *namespace, const char *feature);
extern void write_feature(const char *namespace, const char *feature, float value);
extern void clear_feature(const char *namespace, const char *feature);

void add_interaction(const char *str);

__attribute__ ((visibility ("default")))
int transform(int a, int b) {
    pass_value(a * b);
    char buff[20];
    sprintf(buff, "%s_%d", "gaga", a + b);
    pass_string(buff);
    return a + b;
}

int main(int a, char *b[])
{
    return transform(1,2);
}