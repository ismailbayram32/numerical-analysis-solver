#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#define M_PI 3.14159265358979323846

typedef enum {
    T_NUMBER, T_VARIABLE, T_OPERATOR, T_LOG, T_TRIGO, T_REVTRIGO, T_LPAREN, T_RPAREN
}TokenType;

typedef struct {
    TokenType type;
    char str[10];
    double value;
    double base;
}Token;

//Tokenizer icin gerekli fonksiyonların prototipi
int operatorCtrl(const char);
int functionCtrl(const char*);
int revTrigoCtrl(const char*);
int digitCtrl(const char);
int alphaCtrl(const char);
double my_atof(char*);
int tokenize(char*, Token*);

//Tokenizer'i hesaplanabilir forma sokmak için fonksiyon prototipleri
int oncelik(const char*);
Token* infixToPostfix(Token*, int , int*);
double hesapla(char*, double);

//Sayısal analiz yöntemlerinin prototipleri
void menu();
void bisection();
void regulaFalsi();
void newtonRaphson();
void matrisTersi();
void choleskyALU();
void gaussSeidel();
void sayisalTurev();
void simpson();
void trapez();
void gregoryNewton();

//Yardımcı fonksiyon prototipleri
char* my_getstring();
double turevHesapla(char*,double);
double** matrisAl(int);
void matrisYaz(double**, int);
int diagonalBaskin(double***,double**, int);
void freeMatris(double**, int);
int my_fakt(int);
void newtonFormul(double**, double*, double, int);

int main(void){
    /*char expr[] = "5arcsin1 +1 - x";
    double x = M_PI;
    int size = 0;
    Token tokens[50];
    int num_tokens = tokenize(expr, tokens);

    // Token'ları kontrol et
    for (int i = 0; i < num_tokens; i++) {
        if(tokens[i].type == T_NUMBER)
            printf("Token %d: Type=%d, Str=%s, Value=%.6f\n", i, tokens[i].type, tokens[i].str, tokens[i].value);
        else if(tokens[i].type == T_LOG)
            printf("Token %d: Type=%d, Str=%s, Base=%.6f\n", i, tokens[i].type, tokens[i].str, tokens[i].base);
        else
            printf("Token %d: Type=%d, Str=%s\n", i, tokens[i].type, tokens[i].str);
    }
    Token* postfix = infixToPostfix(tokens, num_tokens, &size);
    for (int i = 0; i < size; i++) {
        printf("%s ", postfix[i].str);
    }
    printf("\nx = %lf\n", x);

    double sonuc = hesapla(expr, x);
    printf("f(%.2lf): %.6f\n", x, sonuc);*/
    menu();
    return 0;
}

int operatorCtrl(const char a){
    return (a == '+' || a == '-' || a == '*' || a == '/' || a == '^');
}
int functionCtrl(const char *s){
    return (strncmp(s, "sin", 3) == 0 || strncmp(s, "cos", 3) == 0 ||
           strncmp(s, "exp", 3) == 0 || strncmp(s, "tan", 3) == 0 || 
           strncmp(s, "cot", 3) == 0 || strncmp(s, "sec", 3) == 0);
}
int revTrigoCtrl(const char* s){
    return (strncmp(s, "arcsin", 6) == 0 || strncmp(s, "arccos", 6) == 0 ||
            strncmp(s, "arctan", 6) == 0 || strncmp(s, "arccot", 6) == 0);
}
int digitCtrl(const char a){
    return ((a >= '0' && a <= '9') || a == 'e' || a == 'p' || a == 'E' || a == 'P');
}
int alphaCtrl(const char a){
    return (a >= 'a' && a <= 'z') || (a >= 'A' && a<= 'Z');
}
double my_atof(char *str){
    double res = 0.0, kesir = 0.0, ondalikBolum = 10.0;
    int dotCtrl = 0, endingCtrl = 1, negativeCtrl = 1;
    if(*str == '-'){
        negativeCtrl = -1;
        str++;
    }
    while (*str && endingCtrl) {
        if (*str == '.') {
            if(dotCtrl == 0)
                dotCtrl = 1;
            else
                endingCtrl = 0; // ikinci nokta, dur
        }
        else if(*str >= '0' && *str <= '9') {
            if (dotCtrl == 0) {
                res = res * 10.0 + (*str - '0');
            } else {
                kesir += (*str - '0') / ondalikBolum;
                ondalikBolum *= 10.0;
            }
        }
        else{
            endingCtrl = 0; // geçerli olmayan karakter, dur
        }

        if (endingCtrl)
            str++;
    }

    return (res + kesir) * negativeCtrl;
}

// İfadeyi token'lara ayırır: sayılar, değişkenler, operatörler, fonksiyonlar, parantezler
int tokenize(char *expr, Token *tokens){
    int tok_count = 0, sub_tok_count = 0, paran_count = 0, sub_length; // Şu ana kadar bulunan token sayilari
    char n = 0, i = 0, *start;
    int len = strlen(expr);
    char *numbuf = (char*)malloc(len + 1);
    char *subexpr = (char*)malloc(len + 1);
    double base = 0.0;

    while(*expr){
        // Boşluk karakterlerini atla
        if(*expr == ' '){
            expr++;
        }
        
        if(*expr == '-'){
            // Önceki token'a bakarak negatif sayı mı yoksa çıkarma operatörü mü olduğunu belirle
            if(tok_count == 0 || tokens[tok_count-1].type == T_OPERATOR || tokens[tok_count-1].type == T_LPAREN){
                // Negatif sayı başlangıcı
                expr++;
                while(*expr == ' ')
                    expr++;
                tokens[tok_count].type = T_NUMBER;
                strcpy(tokens[tok_count].str, "-1");
                tokens[tok_count].value = -1;
                tok_count++;
                tokens[tok_count].type = T_OPERATOR;
                strcpy(tokens[tok_count].str, "*");
                tok_count++;
            }
            else{
                // Çıkarma operatörü
                tokens[tok_count].type = T_OPERATOR;
                strcpy(tokens[tok_count].str, "-");
                tok_count++;
                expr++;
            }
        }
        // Sayılar (negatif olmayan)
        else if((digitCtrl(*expr) || (*expr == '.' && digitCtrl(*(expr + 1)))) && strncmp(expr, "exp", 3) != 0){
            n = 0;
            while((digitCtrl(*expr) || *expr == '.')){
                numbuf[n++] = *expr++;
            }
            if(n > 1 && (numbuf[n-1] == 'e' || numbuf[n-1] == 'E') && digitCtrl(numbuf[n-2])){ //5e gibi örtülü çarpma için
                numbuf[n-1] = '\0';
                tokens[tok_count].type = T_NUMBER;
                strcpy(tokens[tok_count].str, numbuf);
                tokens[tok_count++].value = my_atof(numbuf);
                tokens[tok_count].type = T_OPERATOR;
                strcpy(tokens[tok_count++].str, "*");
                tokens[tok_count].type = T_NUMBER;
                strcpy(tokens[tok_count].str, "e");
                tokens[tok_count++].value = exp(1.0);
            }
            else if(n > 1 && (numbuf[n-1] == 'p' || numbuf[n-1] == 'P') && digitCtrl(numbuf[n-2])){//5p gibi örtülü çarpma için
                numbuf[n-1] = '\0';
                tokens[tok_count].type = T_NUMBER;
                strcpy(tokens[tok_count].str, numbuf);
                tokens[tok_count++].value = my_atof(numbuf);
                tokens[tok_count].type = T_OPERATOR;
                strcpy(tokens[tok_count++].str, "*");
                tokens[tok_count].type = T_NUMBER;
                strcpy(tokens[tok_count].str, "p");
                tokens[tok_count++].value = M_PI;
            }
            else{
                numbuf[n] = '\0';
                tokens[tok_count].type = T_NUMBER;
                strcpy(tokens[tok_count].str, numbuf);
                if(numbuf[0] == 'e' || numbuf[0] == 'E')
                    tokens[tok_count++].value = exp(1.0);
                else if(numbuf[0] == 'p' || numbuf[0] == 'P')
                    tokens[tok_count++].value = M_PI;
                else
                    tokens[tok_count++].value = my_atof(numbuf);
            }
            
            while(*expr == ' ')
                expr++;
            // Örtülü çarpma kontrolü (3x gibi)
            if(*expr == 'x' || *expr == '(' || *expr == 'X' || functionCtrl(expr) || revTrigoCtrl(expr)){
                tokens[tok_count].type = T_OPERATOR;
                strcpy(tokens[tok_count].str, "*");
                tok_count++;
            }
        }
        else if(strncmp(expr, "log_", 4) == 0){
            expr += 4; // "log_" kısmını atla
            tokens[tok_count].type = T_LOG;
            strcpy(tokens[tok_count].str, "log");

            // log_ sonrası sayı (taban) bekleniyor
            base = 0.0;
            while(digitCtrl(*expr) || *expr == 'x' || *expr == 'X'){
                if(*expr == 'x' || *expr == 'X'){
                    tokens[tok_count].base = -1;
                    base = -1; // x degisken olduğu için geçici bir base atadım
                }
                else if(*expr == 'e'){
                    tokens[tok_count].base = exp(1.0);
                    base = exp(1.0);
                }
                else{
                    base = base * 10.0 + (*expr - '0');
                }
                expr++;
            }
            if(base != 1){
                tokens[tok_count].base = (double)base;
            }
            else{
                printf("Gecersiz log tabani.");
                free(numbuf);
                free(subexpr);
                return -1;
            }
            while(*expr == ' '){
                expr++;
            }
            tok_count++;
            // Şimdi y (argüman) kısmını oku
            if(*expr == '('){
                expr++;
                start = expr;
                paran_count = 1;
                tokens[tok_count].type = T_LPAREN;
                strcpy(tokens[tok_count++].str, "(");
                while(*expr && paran_count > 0){
                    if(*expr == '('){
                        paran_count++;
                    }
                    else if(*expr == ')'){
                        paran_count--;
                    }
                    expr++;
                }
                if(paran_count != 0){
                    printf("Parantez dengesi bozuk.\n");
                    return -1;
                }
                sub_length = expr - start - 1;
                strncpy(subexpr, start, sub_length);
                subexpr[sub_length] = '\0';

                Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));  // İç fonksiyonu tokenize etmek için geçici bir token dizisi
                sub_tok_count = tokenize(subexpr, sub_tokens);

                if(sub_tok_count == -1){
                    printf("Hatali fonksiyon içeriği.\n");
                    free(numbuf);
                    free(subexpr);
                    free(sub_tokens);
                    return -1;
                }

                // Sub-tokenları ana token dizisine aktar
                for(i = 0; i < sub_tok_count; i++){
                    tokens[tok_count++] = sub_tokens[i];
                }
                tokens[tok_count].type = T_RPAREN;
                strcpy(tokens[tok_count++].str, ")");
                free(sub_tokens);
            }
        }
        // Harf ile başlayan bir şey: ya 'x' değişkeni ya da fonksiyon
        else if(alphaCtrl(*expr)){
            if(*expr == 'x' || *expr == 'X'){
                // Değişken 'x'
                tokens[tok_count].type = T_VARIABLE;
                strcpy(tokens[tok_count].str, "x");
                expr++;
                tok_count++;
            } 
            else if(functionCtrl(expr)){
                // Fonksiyon ismi (ilk 3 karakteri oku)
                char trigo[4] = {expr[0], expr[1], expr[2], '\0'};
                expr += 3;
                tokens[tok_count].type = T_TRIGO;
                strcpy(tokens[tok_count].str, trigo);
                tok_count++;

                while(*expr == ' '){
                    expr++;
                }
                if(*expr == '('){
                    // Parantezli kullanım
                    expr++;
                    start = expr;
                    paran_count = 1;
                    tokens[tok_count].type = T_LPAREN;
                    strcpy(tokens[tok_count++].str, "(");
                    while (*expr && paran_count > 0){
                        if(*expr == '('){
                            paran_count++;
                        }
                        else if(*expr == ')'){
                            paran_count--;
                        }
                        expr++;
                    }
                    if(paran_count != 0){
                        printf("Parantez hatasi.\n");
                        return -1;
                    }
                    sub_length = expr - start - 1;
                    strncpy(subexpr, start, sub_length);
                    subexpr[sub_length] = '\0';

                    Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));
                    sub_tok_count = tokenize(subexpr, sub_tokens);

                    if(sub_tok_count == -1) 
                        return -1;
                    for(int i = 0; i < sub_tok_count; i++){
                        tokens[tok_count++] = sub_tokens[i];
                    }
                    tokens[tok_count].type = T_RPAREN;
                    strcpy(tokens[tok_count++].str, ")");
                    free(sub_tokens);
                }
                else{
                    i = 0;
                    while(expr[i] && expr[i] != ' ' && expr[i] != ')' && expr[i] != '(' && !operatorCtrl(expr[i])){ 
                        i++;
                    }
                    strncpy(subexpr, expr, i);
                    subexpr[i] = '\0';
                    expr += i;

                    Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));
                    sub_tok_count = tokenize(subexpr, sub_tokens);
                    if(sub_tok_count == -1) 
                        return -1;
                    tokens[tok_count].type = T_LPAREN;
                    strcpy(tokens[tok_count++].str, "(");
                    for(i = 0; i < sub_tok_count; i++){
                        tokens[tok_count++] = sub_tokens[i];
                    }
                    tokens[tok_count].type = T_RPAREN;
                    strcpy(tokens[tok_count++].str, ")");
                    free(sub_tokens);
                }
            }
            else if(revTrigoCtrl(expr)){
                char revtrigo[7] = {expr[0], expr[1], expr[2], expr[3], expr[4], expr[5], '\0'};
                expr += 6;
                tokens[tok_count].type = T_REVTRIGO;
                strcpy(tokens[tok_count].str, revtrigo);
                tok_count++;

                while(*expr == ' '){
                    expr++;
                }
                if(*expr == '('){
                    // Parantezli kullanım
                    expr++;
                    start = expr;
                    paran_count = 1;
                    tokens[tok_count].type = T_LPAREN;
                    strcpy(tokens[tok_count++].str, "(");
                    while(*expr && paran_count > 0){
                        if(*expr == '('){
                            paran_count++;
                        }
                        else if(*expr == ')'){
                            paran_count--;
                        }
                        expr++;
                    }
                    if(paran_count != 0){
                        printf("Parantez hatasi.\n");
                        return -1;
                    }
                    sub_length = expr - start - 1;
                    strncpy(subexpr, start, sub_length);
                    subexpr[sub_length] = '\0';

                    Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));
                    sub_tok_count = tokenize(subexpr, sub_tokens);

                    if(sub_tok_count == -1) 
                        return -1;

                    for(int i = 0; i < sub_tok_count; i++){
                        tokens[tok_count++] = sub_tokens[i];
                    }
                    tokens[tok_count].type = T_RPAREN;
                    strcpy(tokens[tok_count++].str, ")");
                    free(sub_tokens);
                }
                else{
                    i = 0;
                    while(expr[i] && expr[i] != ' ' && expr[i] != ')' && expr[i] != '(' && !operatorCtrl(*(expr+i))) 
                        i++;
                    strncpy(subexpr, expr, i);
                    subexpr[i] = '\0';
                    expr += i;

                    Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));
                    sub_tok_count = tokenize(subexpr, sub_tokens);
                    if(sub_tok_count == -1) 
                        return -1;
                    tokens[tok_count].type = T_LPAREN;
                    strcpy(tokens[tok_count++].str, "(");
                    for(i = 0; i < sub_tok_count; i++) {
                        tokens[tok_count++] = sub_tokens[i];
                    }
                    tokens[tok_count].type = T_RPAREN;
                    strcpy(tokens[tok_count++].str, ")");
                    free(sub_tokens);
                }
            }
            else{
                // Tanınmayan string varsa hata ver
                printf("Bilinmeyen ifade: %s\n", expr);
                return -1;
            }
        }
        // Operatör (+ - * / ^)
        else if(operatorCtrl(*expr)){
            tokens[tok_count].type = T_OPERATOR;
            tokens[tok_count].str[0] = *expr;
            tokens[tok_count].str[1] = '\0'; // string sonlandır
            expr++;
            tok_count++;

            while(*expr == ' ')
                expr++; // boşlukları geç
            if(*expr == '('){
                paran_count = 1;
                i = 0;
                expr++; // '(' karakterini geç
                tokens[tok_count].type = T_LPAREN;
                strcpy(tokens[tok_count++].str, "(");
                while(*expr && paran_count > 0){
                    if(*expr == '('){
                        paran_count++;
                    }
                    else if(*expr == ')'){
                        paran_count--;
                    }
                    if(paran_count > 0)
                        subexpr[i++] = *expr;
                    expr++;
                }
                subexpr[i] = '\0';

                // Parantezli ifadeyi tekrar tokenize et
                Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));
                sub_tok_count = tokenize(subexpr, sub_tokens);
                if(sub_tok_count == -1){
                    printf("Parantezli ifade hatali.\n");
                    free(numbuf);
                    free(subexpr);
                    free(sub_tokens);
                    return -1;
                }
                for(i = 0; i < sub_tok_count; i++)
                    tokens[tok_count++] = sub_tokens[i];
                tokens[tok_count].type = T_RPAREN;
                strcpy(tokens[tok_count++].str, ")");
                free(sub_tokens);
            }
        }
        // Açılış parantezi
        else if(*expr == '('){
            tokens[tok_count].type = T_LPAREN;
            strcpy(tokens[tok_count++].str, "(");
            expr++;
            start = expr;
            paran_count = 1;
            while (*expr && paran_count > 0){
                if(*expr == '(') paran_count++;
                else if(*expr == ')') paran_count--;
                expr++;
            }
            if(paran_count != 0) return -1;

            sub_length = expr - start - 1;
            strncpy(subexpr, start, sub_length);
            subexpr[sub_length] = '\0';

            Token *sub_tokens = (Token*)malloc(sizeof(Token) * (strlen(subexpr) + 1));
            sub_tok_count = tokenize(subexpr, sub_tokens);

            if(sub_tok_count == -1) return -1;

            for(i = 0; i < sub_tok_count; i++){
                tokens[tok_count++] = sub_tokens[i];
            }
            tokens[tok_count].type = T_RPAREN;
            strcpy(tokens[tok_count++].str, ")");
            free(sub_tokens);
        }
        else if(*expr == ')'){
            tokens[tok_count].type = T_RPAREN;
            strcpy(tokens[tok_count++].str, ")");
            expr++;
        }
        // Geçersiz karakter durumu
        else{
            printf("Geçersiz karakter: %c\n", *expr);
            free(numbuf);
            free(subexpr);
            return -1;
        }
    }
    free(numbuf); free(subexpr);
    return tok_count; //Toplam bulunan token sayısını döndür
}

int oncelik(const char* operator){
    if (strcmp(operator, "^") == 0) {
        return 3; // En yüksek öncelik
    } else if (strcmp(operator, "*") == 0 || strcmp(operator, "/") == 0) {
        return 2; // Orta öncelik
    } else if (strcmp(operator, "+") == 0 || strcmp(operator, "-") == 0) {
        return 1; // En düşük öncelik
    }
    return 0; // Operatör değilse, 0 döner
}

Token *infixToPostfix(Token *infix, int n, int *postfix_size){
    Token *stack = (Token *)malloc(sizeof(Token) * n);
    Token *postfix = (Token *)malloc(sizeof(Token) * n);
    int top = -1, j = 0;

    for(int i = 0; i < n; i++){
        Token token = infix[i];
        
        if(token.type == T_NUMBER || token.type == T_VARIABLE){
            postfix[j++] = token;
        }
        else if(token.type == T_TRIGO || token.type == T_REVTRIGO || token.type == T_LOG){
            stack[++top] = token;
        }
        else if(token.type == T_OPERATOR){
            while(top != -1 && stack[top].type == T_OPERATOR && oncelik(stack[top].str) >= oncelik(token.str)){
                postfix[j++] = stack[top--];
            }
            stack[++top] = token;
        }
        else if(token.type == T_LPAREN){
            stack[++top] = token;
        }
        else if(token.type == T_RPAREN){
            while(top != -1 && stack[top].type != T_LPAREN){
                postfix[j++] = stack[top--];
            }
            if(top == -1){
                printf("Hata: Parantez uyumsuzluğu.\n");
                free(stack);
                free(postfix);
                return NULL;
            }
            top--; // '(' parantezini at

            if(top != -1 && (stack[top].type == T_TRIGO || stack[top].type == T_REVTRIGO || stack[top].type == T_LOG)){
                postfix[j++] = stack[top--];
            }
        }
    }
    while(top != -1){
        if(stack[top].type == T_LPAREN || stack[top].type == T_RPAREN){
            printf("Hata: Parantez dengesi bozuk.\n");
            free(stack);
            free(postfix);
            return NULL;
        }
        postfix[j++] = stack[top--];
    }

    free(stack);
    *postfix_size = j;
    return postfix;
}

double hesapla(char *expr, double x) {
    int postfix_size = 0, top = -1, i, num_token = 0;
    double a, b, result, final;
    Token* infix = (Token*)malloc(strlen(expr) * sizeof(Token) * 2);
    num_token = tokenize(expr, infix);
    Token *postfix = infixToPostfix(infix, num_token, &postfix_size);
    if (!postfix) {
        printf("Postfix donusumunde hata olustu.\n");
        return 0.0;
    }

    double *stack = (double *)malloc(sizeof(double) * postfix_size);

    for(i = 0; i < postfix_size; i++){
        Token token = postfix[i];
        if(token.type == T_NUMBER){
            stack[++top] = token.value;
        }
        else if(token.type == T_VARIABLE){
            stack[++top] = x;
        }
        else if(token.type == T_OPERATOR){
            b = stack[top--];
            a = stack[top--];
            result = 0.0;
            switch(token.str[0]){
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = a / b; break;
                case '^': result = pow(a, b); break;
                default:
                    printf("Bilinmeyen operator: %c\n", token.str[0]);
                    free(stack);
                    free(postfix);
                    return 0.0;
            }
            stack[++top] = result;
        }
        else if(token.type == T_TRIGO || token.type == T_REVTRIGO || token.type == T_LOG){
            a = stack[top--];
            result = 0.0;
            if(token.type == T_TRIGO){
                if(strcmp(token.str, "sin") == 0) result = sin(a);
                else if(strcmp(token.str, "cos") == 0) result = cos(a);
                else if(strcmp(token.str, "tan") == 0) result = tan(a);
                else if(strcmp(token.str, "cot") == 0) result = 1.0 / tan(a);
                else if(strcmp(token.str, "sec") == 0) result = 1.0 / cos(a);
                else if(strcmp(token.str, "exp") == 0) result = exp(a);
            }
            else if(token.type == T_REVTRIGO){
                if(strcmp(token.str, "arcsin") == 0) result = asin(a);
                else if(strcmp(token.str, "arccos") == 0) result = acos(a);
                else if(strcmp(token.str, "arctan") == 0) result = atan(a);
                else if(strcmp(token.str, "arccot") == 0) result = atan(1.0 / a);
            }
            else if(token.type == T_LOG){
                if(token.base <= 0)
                    result = log(a) / log(x);
                else
                    result = log(a) / log(token.base);
            }
            stack[++top] = result;
        }
    }

    final = (top == 0) ? stack[top] : 0.0;
    free(stack);
    free(postfix);
    free(infix);
    return final;
}

void menu(){
    int num = 1;
    printf("\n***Hosgeldiniz***\n1.Bisection Yontemi\n2.Regula Falsi Yontemi\n3.Newton Raphson Yontemi\n4.Matrisin Tersi\n5.Cholesky(ALU) Yontemi\n");
    printf("6.Gauss Seidel Yontemi\n7.Sayisal Turev Yontemi\n8.Simpson Kurali\n9.Trapez Kurali\n10.Gregory Newton Enterpolasyonu\n0.Exit\n");
    printf("NOT: Pi sayisi icin 'p' veya 'P' karakterini kullaniniz.\n");
    while(num != 0){
        printf("Yontem seciniz(Cikmak icin '0'a basiniz): ");
        scanf("%d", &num);
        switch (num)
        {
        case 0:
            printf("Program Sonlandiriliyor..."); break;
        case 1:
            bisection(); break;
        case 2:
            regulaFalsi(); break;
        case 3:
            newtonRaphson(); break;
        case 4:
            matrisTersi(); break;
        case 5:
            choleskyALU(); break;
        case 6:
            gaussSeidel(); break;
        case 7:
            sayisalTurev(); break;
        case 8:
            simpson(); break;
        case 9:
            trapez(); break;
        case 10:
            gregoryNewton(); break;
        default:
            break;
        }
    }
}
void bisection(){
    char* expr;
    double x1, x2, epsilon, c = 0, prev_c = 0, resultX1, resultX2, resultC = 1, error = 100;
    int n = 0;
    printf("\nBisection:\n");
    expr = my_getstring();
    printf("(x1 x2) araligini giriniz:");
    do{
        scanf("%lf %lf", &x1, &x2);
        resultX1 = hesapla(expr, x1);
        resultX2 = hesapla(expr, x2);
        printf("f(x1) = %lf, f(x2) = %lf\n", resultX1, resultX2);
        if(resultX1*resultX2 > 0)
            printf("Girdiginiz aralikta bu yontem kullanilamaz, baska degerler giriniz:");
    }while(resultX1*resultX2 > 0);
    printf("Hata payini giriniz:");
    scanf("%lf", &epsilon);

    while(error >= epsilon && resultC != 0){
        resultX1 = hesapla(expr, x1);
        resultX2 = hesapla(expr, x2);

        if(resultX1*resultX2 < 0){
            prev_c = c;
            c=(x1+x2)/2;
            resultC = hesapla(expr, c);
            if(resultC == 0){
                x1 = c, x2 = c;
            }
            else if(resultX1*resultC < 0){
                x2 = c;
            }
            else if(resultX2*resultC < 0){
                x1 = c;
            }
            error = fabs(c-prev_c);
        }
        else{
            if(resultX1 == 0)
                c = x1;
            else if(resultX2 == 0)
                c = x2;
            error = 0.0;
        }
        if(resultC == 0){
            error = 0;
        }
        n++;
        printf("%d. iterasyon: c = %lf, f(c) = %lf, hata = %lf\n", n, c, resultC, error);
    }
    printf("Kok: %lf\n", c);
    free(expr);
}
void regulaFalsi(){
    char* expr;
    double x1, x2, epsilon, c = 0, prev_c = 0, resultX1, resultX2, resultC = 1, error = 100;
    int n = 0;
    printf("\nRegula Falsi:\n");
    expr = my_getstring();
    printf("(x1 x2) araligini giriniz:");
    do{
        scanf("%lf %lf", &x1, &x2);
        resultX1 = hesapla(expr, x1);
        resultX2 = hesapla(expr, x2);
        printf("f(x1) = %lf, f(x2) = %lf\n", resultX1, resultX2);
        if(resultX1*resultX2 > 0)
            printf("Girdiginiz aralikta bu yontem kullanilamaz, baska degerler giriniz:");
    }while(resultX1*resultX2 > 0);
    printf("Hata payini giriniz:");
    scanf("%lf", &epsilon);

    while(error >= epsilon && resultC != 0){
        resultX1 = hesapla(expr, x1);
        resultX2 = hesapla(expr, x2);
        if(resultX1*resultX2 < 0){
            prev_c = c;
            c=((x1*resultX2)-(x2*resultX1))/(resultX2-resultX1);
            resultC = hesapla(expr, c);
            if(resultC == 0){
                x1 = c;
                x2 = c;
            }
            else if(resultX1*resultC < 0){
                x2 = c;
            }
            else if(resultX2*resultC < 0){
                x1 = c;
            }
            error=fabs(c-prev_c);
        }
        else{
            if(resultX1 == 0)
                c = x1;
            else if(resultX2 == 0)
                c = x2;
            error = 0.0;
            resultC = hesapla(expr, c);
        }
        if(resultC == 0){
            error = 0;
        }
        n++;
        printf("%d. iterasyon: c = %lf, f(c) = %lf, hata = %lf\n", n, c, resultC, error);
    }
    printf("Kok: %lf\n", c);
    free(expr);
}
void newtonRaphson(){
    char* expr;
    double x, x0, epsilon, result = 1, error = 100;
    int n = 0;
    printf("\nNewton Raphson:\n");
    expr = my_getstring();
    printf("x0 degerini giriniz: ");
    scanf("%lf", &x0);
    printf("Hata payini giriniz: ");
    scanf("%lf", &epsilon);
    while(error >= epsilon && n < 500 && result != 0){
        if(turevHesapla(expr, x0) == 0){
            printf("Turev degeri sifir olamaz!\n");
            return;
        }
        x = x0 - (hesapla(expr, x0)/turevHesapla(expr, x0));
        result = hesapla(expr, x);
        error = fabs(x-x0);
        if(fabs(result) < 1e-6){
            result = 0;
            error = 0;
        }
        x0 = x;
        n++;
        printf("%d. iterasyon: x = %lf, f(x) = %lf, hata = %lf\n", n, x, result, error);
    }
    printf("Kok: %lf\n", x);
    free(expr);
}
void matrisTersi(){
    double** mtr, **birimMtr, *tempMtr, *brmTempMtr, pivot;
    int n, i, j, k;
    printf("\nMatris boyutunu giriniz(NxN): ");
    scanf("%d", &n);
    birimMtr = (double**)calloc(n, sizeof(double*));
    tempMtr = (double*)calloc(n, sizeof(double));
    brmTempMtr = (double*)calloc(n, sizeof(double));
    for(i = 0; i < n; i++){
        birimMtr[i] = (double*)calloc(n, sizeof(double));
        birimMtr[i][i] = 1;
    }
    mtr = matrisAl(n);
    for(i = 0; i < n; i++){
        if(mtr[i][i] == 0){ //Pivotun sıfır olması durumunda satır değişimi
            for(j = i+1; j < n; j++){
                if(mtr[j][i] != 0){
                    for(k = 0; k < n; k++){
                        tempMtr[k] = mtr[i][k];
                        mtr[i][k] = mtr[j][k];
                        mtr[j][k] = tempMtr[k];

                        brmTempMtr[k] = birimMtr[i][k];
                        birimMtr[i][k] = birimMtr[j][k];
                        birimMtr[j][k] = brmTempMtr[k];
                    }
                    j = n;
                }
            }
        }
        pivot = mtr[i][i];
        if(pivot == 0){
            printf("Matrisin tersi yok(det = 0)!\n");
            return;
        }
        else{
            for(j = 0; j < n; j++){
                mtr[i][j] /= pivot;
                birimMtr[i][j] /= pivot;
            }
            for(j = 0; j < n; j++){
                pivot = mtr[j][i];
                if(j != i){
                    for(k = 0; k < n; k++){
                        mtr[j][k] -= mtr[i][k] * pivot;
                        birimMtr[j][k] -= birimMtr[i][k] * pivot;
                    }
                }
            }
        }
    }
    printf("Ters matris:\n");
    matrisYaz(birimMtr, n);

    for(i = 0; i < n; i++){
        free(mtr[i]);
        free(birimMtr[i]);
    }
    free(mtr), free(tempMtr);
    free(birimMtr), free(brmTempMtr);
}
void choleskyALU(){
    int n, i, j, k;
    double** A, **L, **U, sum, *y, *b, *x;
    printf("\nCholesky ALU Metodu:\n");
    printf("Matrisin boyutunu giriniz: ");
    scanf("%d", &n);
    A = matrisAl(n);
    L = (double**)calloc(n, sizeof(double*));
    U = (double**)calloc(n, sizeof(double*));
    y = (double*)calloc(n, sizeof(double));
    b = (double*)calloc(n, sizeof(double));
    x = (double*)calloc(n, sizeof(double));
    printf("Ax = b denklemindeki 'b' dizisini giriniz:\n");
    for(i = 0; i < n; i++){
        scanf("%lf", &b[i]);
    }
    for(i = 0; i < n; i++){
        L[i] = (double*)calloc(n, sizeof(double));
        U[i] = (double*)calloc(n, sizeof(double));
    }
    for(i = 0; i < n; i++){
        // L matrisini hesapla
        for(k = i; k < n; k++){
            sum = 0;
            for(j = 0; j < i; j++)
                sum += L[k][j] * U[j][i];
            L[k][i] = A[k][i] - sum;
        }
    
        // U matrisini hesapla
        for(k = i; k < n; k++){
            if (L[i][i] == 0){
                printf("Sifira bolme hatasi! LU ayristirmasi yapilamaz.\n");
                freeMatris(A, n);
                freeMatris(L, n);
                freeMatris(U, n);
                return;
            }
            if(i == k)
                U[i][k] = 1.0; // Diagonal elemanı sabit 1 yap
            else{
                sum = 0;
                for(j = 0; j < i; j++)
                    sum += L[i][j] * U[j][k];
                U[i][k] = (A[i][k] - sum) / L[i][i];
            }
        }
    }
    for(i = 0; i < n; i++){ //L.y = b denklemi çözümü
        sum = 0;
        for(j = 0; j < i; j++){
            sum += L[i][j] * y[j];
        }
        y[i] = (b[i] - sum) / L[i][i];
    }
    for(i = n - 1; i >= 0; i--){ //U.x = y denklemi çözümü
        sum = 0;
        for(j = i + 1; j < n; j++){
            sum += U[i][j] * x[j];
        }
        x[i] = y[i] - sum;
    }
    printf("L:\n");
    matrisYaz(L, n);
    printf("U:\n");
    matrisYaz(U, n);
    printf("Cozum:\n");
    for(i = 0; i < n; i++){
        printf("%lf\n", x[i]);
    }
    printf("\n");

    freeMatris(A,n);
    freeMatris(L,n);
    freeMatris(U,n);
    free(y), free(b), free(x);
}
void gaussSeidel(){
    int n, i, j, k, count = 0;
    double **mtr, *sonuc, *degisken, *prev_deg, *error, epsilon, maxError = 100;
    printf("\nGauss Seidel Yontemi:\n");
    printf("Denklem sayisini giriniz: ");
    scanf("%d", &n);
    sonuc = (double*)calloc(n, sizeof(double));
    degisken = (double*)calloc(n, sizeof(double));
    prev_deg = (double*)calloc(n, sizeof(double));
    error = (double*)calloc(n, sizeof(double));
    mtr = matrisAl(n);
    printf("Sonuc matrisini giriniz:\n");
    for(i = 0; i < n; i++){
        scanf("%lf", &sonuc[i]);
    }
    if(diagonalBaskin(&mtr, &sonuc, n) == 0){
        printf("Matris diagonal baskin yapilamiyor.\n");
        return;
    }
    for(i = 0; i < n; i++){ // matrisi yaz
        for(j = 0; j < n; j++){
            printf("%lf ", *(*(mtr+i)+j));
        }
        printf("| %lf\n", sonuc[i]);
    }

    printf("Hata payini giriniz: ");
    scanf("%lf", &epsilon);

    printf("Baslangic iterasyonunu giriniz(x1 x2 ... xn): ");
    for(i = 0; i < n; i++){
        scanf(" %lf", &degisken[i]);
    }
    while(maxError >= epsilon){
        count++;
        for(i = 0; i < n; i++){
            prev_deg[i] = degisken[i];
            degisken[i] = sonuc[i] / mtr[i][i];
            for(j = 0; j < n; j++){
                if(i != j)
                    degisken[i] = (degisken[i] - (mtr[i][j] * degisken[j] / mtr[i][i]));
            }
            error[i] = fabs(degisken[i] - prev_deg[i]);
            printf("%d. iterasyon: x[%d] = %lf Hata: %lf\n", count, i+1, degisken[i], error[i]);
        }
        printf("\n");
        maxError = error[0];
        for(k = 0; k < n; k++){
            if(maxError <= error[k]){
                maxError = error[k];
            }
        }
    }
    for(i = 0; i < n; i++){
        free(mtr[i]);
    }
    free(mtr), free(degisken), free(prev_deg), free(sonuc), free(error);
}
void sayisalTurev(){
    char* expr, ctrl;
    double h, x, ileri, merkez, geri;
    printf("\nSayisal Turev:\n");
    expr = my_getstring();
    do{
        printf("h degerini giriniz(h ne kadar kucukse o kadar yakin sonuc): ");
        scanf("%lf", &h);
        if(h == 0)
            printf("h degeri 0 olamaz, tekrar giriniz: ");
    }while(h == 0);
    do{
        printf("Turevini hesaplamak istediginiz noktayi giriniz: ");
        scanf("%lf", &x);
        
        ileri = (1/(2*h))*(-3*hesapla(expr,x) + 4*hesapla(expr,x+h) - hesapla(expr, x+2*h));
        merkez = (1/(2*h))*(hesapla(expr, x+h)-hesapla(expr, x-h));
        geri = (1/(2*h))*(3*hesapla(expr,x) - 4*hesapla(expr,x-h) + hesapla(expr, x-2*h));

        printf("f'(%lf):\nIleri fark: %lf\nMerkezi fark: %lf\nGeri fark: %lf\n", x, ileri, merkez, geri);
        printf("Baska bir x degeri hesaplamak ister misiniz? (y/n): ");
        scanf(" %c", &ctrl); 
    }while(ctrl == 'y' || ctrl == 'Y');
    free(expr);
}
void simpson(){
    char *expr;
    double h, a, b, sonuc = 0, fa, fb, i;
    int n, j = 1, tercih;
    printf("\nSimpson Yontemi:\n");
    expr = my_getstring();
    printf("Integral araligini giriniz(a b): ");
    scanf("%lf %lf", &a, &b);
    fa = hesapla(expr, a);
    fb = hesapla(expr, b);
    printf("1.Simpson 1/3\n2.Simpson 3/8\nHangi simpson yontemini kullanmak istiyorsun: ");
    scanf("%d", &tercih);
    switch (tercih)
    {
    case 1:
        printf("Aralik sayisini giriniz(n:cift olmak zorunda): ");
        do{
            scanf("%d", &n);
            if(n % 2 == 1)
                printf("Hata, n sayisini n=2k olarak giriniz: ");
        }while(n % 2 == 1);
        h = fabs(b-a)/n;
        sonuc = h*(fa+fb)/3;
        for(i = a+h; i < b; i += h){
            if(j % 2 == 1)
                sonuc += h / 3 * 4 * hesapla(expr, i);
            else
                sonuc += h / 3 * 2 * hesapla(expr, i);
            j++;
        }
        printf("Simpson 1/3 yontemi ile integral sonucu: %lf\n", sonuc);
        break;
    case 2:
        printf("Aralik sayisini giriniz(n: 3'un kati olmak zorunda): ");
        do{
            scanf("%d", &n);
            if(n % 3 != 0)
                printf("Hata, n sayisini n=3k olarak giriniz: ");
        }while(n % 3 != 0);
        h = fabs(b-a)/n;
        sonuc = 3*h*(fa+fb)/8;
        for(i = a+h; i < b; i += h){
            if(j % 3 == 0)
                sonuc += 2 * 3 * h / 8 * hesapla(expr, i);
            else
                sonuc += 3 * 3 * h / 8 * hesapla(expr, i);
            j++;
        }
        printf("Simpson 3/8 yontemi ile integral sonucu: %lf\n", sonuc);
        break;
    default:
        break;
    }
    free(expr);
}
void trapez(){
    char* expr;
    double h, a, b, sonuc = 0, fa, fb, i;
    int n;
    printf("\nTrapez Yontemi:\n");
    expr = my_getstring();
    printf("Integral araligini giriniz(a b): ");
    scanf("%lf %lf", &a, &b);
    fa = hesapla(expr, a);
    fb = hesapla(expr, b);
    printf("Yamuk sayisini giriniz(n):");
    scanf("%d", &n);
    h = fabs(b-a)/n;
    sonuc = (fa+fb)/2;
    for(i = a+h; i < b; i += h){
        sonuc += hesapla(expr, i);
    }
    sonuc *= h;
    printf("Integral sonucu: %lf\n", sonuc);
    free(expr);
}
void gregoryNewton(){
    int n, i, j;
    double *x, **fx, h, s, sonuc, a, temp;
    printf("\nGregory Newton Enterpolasyonu:\n");
    printf("Veri sayisini giriniz(n): ");
    scanf("%d", &n);
    x = (double*)calloc(n, sizeof(double));
    fx = (double**)calloc(n, sizeof(double*));
    printf("h degerini giriniz: ");
    scanf("%lf", &h);
    for(i = 0; i < n; i++){
        fx[i] = (double*)calloc(n, sizeof(double));
        printf("x%d ve f(x%d) degerini giriniz: ", i+1, i+1);
        scanf("%lf %lf", &x[i], &fx[0][i]);
        
        if(i > 0 && h != (x[i] - x[i-1])){
            printf("x'ler arasindaki fark h kadar olmak zorunda!\n");
            i--;
        }
    }
    for(i = 0; i < n-1; i++){
        for(j = 1; j < n-i; j++){
            fx[i+1][j-1] = fx[i][j] - fx[i][j-1];
        }
    }
    printf("Fark Tablosu:\n");
    matrisYaz(fx, n);
    newtonFormul(fx, x, h, n);
    do{
        printf("Hesaplamak istediginiz x degerini giriniz(Cikmak icin 999 giriniz): ");
        scanf("%lf", &a);
        s = (a - x[0]) / h;
        sonuc = fx[0][0];
        temp = 1;
        for(i = 1; i < n; i++){
            temp *= (s - (i-1));
            sonuc += temp * fx[i][0] / my_fakt(i);
        }
        printf("Sonuc: %lf\n", sonuc); 
    }while(a != 999);

    freeMatris(fx, n);
    free(x);
}

char* my_getstring(){
    int i = 0;
    char c = 0;
    char* str = (char*)malloc(1 * sizeof(char));
    printf("Denklemi giriniz: ");
    do{
        scanf("%c", &c);
    }while(c == '\n');
    
    do{
        str[i++] = c;
        str = (char*)realloc(str, (i+1) * sizeof(char));
        scanf("%c", &c);
    }while(c != '\n');
    str[i] = '\0';
    return str;
}
double turevHesapla(char* expr, double x){
    double h = 0.000001;
    return ((hesapla(expr, x + h)-hesapla(expr, x - h)) / (2.0 * h));
}
double** matrisAl(int n){
    int i, j;
    double **mtr = (double**)calloc(n, sizeof(double*));
    printf("Matrisi giriniz:\n");
    for(i = 0; i < n; i++){
        mtr[i] = (double*)calloc(n, sizeof(double));
        for(j = 0; j < n; j++){
            scanf("%lf", (*(mtr+i)+j));
        }
    }
    return mtr;
}
void matrisYaz(double** mtr, int n){
    int i, j;
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            printf("%lf ", *(*(mtr+i)+j));
        }
        printf("\n");
    }
}
int diagonalBaskin(double ***mtr, double **sonuc, int n){
    int i, j, k, *kullanim, uygun, *perm;
    double **A = *mtr, **tempA;
    double *b = *sonuc, *tempb;
    double satirToplam, diagDeger;

    // Satır atamaları ve permütasyon dizisi
    kullanim = (int*)calloc(n, sizeof(int));
    perm = (int*)malloc(n * sizeof(int));

    for(i = 0; i < n; i++){
        perm[i] = -1;
    }
    for(i = 0; i < n; i++){
        uygun = 0;
        j = 0;
        while(j < n){
            if(kullanim[j] == 0){
                diagDeger = fabs(A[j][i]);
                satirToplam = 0.0;
                for(k = 0; k < n; k++){
                    if(k != i) 
                        satirToplam += fabs(A[j][k]);
                }

                if(diagDeger >= satirToplam){
                    perm[i] = j;
                    kullanim[j] = 1;
                    uygun = 1;
                    j = n;  // while'dan çıkmak için
                }
                else{
                    j++;
                }
            }
            else{
                j++;
            }
        }

        if(uygun == 0){
            free(kullanim);
            free(perm);
            return 0;  // Diagonal baskın hale getirilemez
        }
    }

    // Matris ve sonuç vektörünü yeni sıraya göre yeniden düzenle
    tempA = (double**)malloc(n * sizeof(double*));
    tempb = (double*)malloc(n * sizeof(double));
    for(i = 0; i < n; i++){
        tempA[i] = (double*)malloc(n * sizeof(double));
        for(j = 0; j < n; j++){
            tempA[i][j] = A[perm[i]][j];
        }
        tempb[i] = b[perm[i]];
    }

    // Eski matrise geri kopyala
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            A[i][j] = tempA[i][j];
        }
        b[i] = tempb[i];
    }

    for(i = 0; i < n; i++){
        free(tempA[i]);
    }
    free(tempA);
    free(tempb);
    free(kullanim);
    free(perm);

    return 1;
}
void freeMatris(double** mtr, int n){
    int i;
    for(i = 0; i < n; i++){
        free(mtr[i]);
    }
    free(mtr);
}
int my_fakt(int x){
    int i, sonuc = 1;
    for(i = 1; i <= x; i++){
        sonuc *= i;
    }
    return sonuc;
}
void newtonFormul(double** fx, double* x, double h, int n){
    int i, j;
    double delta;
    printf("f(x) = %.4lf", fx[0][0]); // f(x0)

    for(i = 1; i < n; i++){
        // Katsayı: ∆^i f(x0)
        delta = fx[i][0];
        if (delta >= 0)
            printf(" + ");
        else
            printf(" - ");

        // Mutlak değeri yaz
        printf("%.4lf", delta < 0 ? -delta : delta);

        // s çarpanları
        printf(" * s");
        for(j = 1; j < i; j++){
            printf("*(s-%d)", j);
        }

        // Bölü faktöriyel
        printf("/%d!", i);
    }
    printf("\ns = (x-%.2lf)/%.2lf", x[0], h);
    printf("\n");
}