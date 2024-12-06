const int a = 10, ty=90;
const int b[3] = {1, 2, 3};
int x = 5, z=114514;
int y[3];
int global_var = 0;
char buaa='\0';
const char aa[5]="abc\0";
char aaa[6]="xyz";

int g(int arr[]) {
    return arr[0]+arr[arr[1]+-arr[0]];
}

char foo(int aa, int bb){
    return 'o'; /*nothing is true*/
}

void fooo(int aa, int bb){
    return;
}

int func() {
    global_var = global_var + 1;
    return 1;
}
int main() {
    printf("21374067\n");
    int i = 0;
    char c = 'a';
    const char ch = 'b';

    int gi;
    int array[3] = {1, 2, 3};
    gi = getint();

    i = g(array);
    c = getchar();

    for (c='a'; c < (127 % 128) || i != 0; c = c+1) {
        c=c+1;
        if (c=='x'){
            break;
        }
    }
    for (c='a';; c = c+1) {
        c=c+1;
        if (c=='x'){
            break;
        }
    }


    printf("%c\n",c);
    printf("%d\n",c);
    return 0;
}
