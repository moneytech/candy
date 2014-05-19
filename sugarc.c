#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#endif

unsigned char buf[256];
unsigned char buf_start = 255;

void fillBuf() {
    if (!fread(buf, 1, 255, stdin)) {
        exit(0);
    }
}

unsigned char nthByte(unsigned char i) {
    return buf[(unsigned char)(buf_start + i)];
}

unsigned char getNextByte() {
    unsigned char byte[2];

    buf_start++;

    if (!fread(byte, 1, 1, stdin)) {
        buf[(unsigned char)(buf_start - 1)] = 0x00;
    }
     else {
        buf[(unsigned char)(buf_start - 1)] = byte[0];
    }

    if (!buf[buf_start]) {
        return 0;
    }
     else {
        return 1;
    }
}

int nextLineIndent() {
    for (unsigned char i = 0; i < 255; i++) {
        if (nthByte(i) == '\n' && nthByte(i+1) != '\n') {
            unsigned char count;
            for (count = 0; nthByte(i + count + 1) == ' '; count++);
            return count >> 2;
        }
    }

    return 0;
}

int main (int argc, char **argv) {
    int line_pos = 0;
    unsigned char open_braces = 0;
    unsigned char previous_indent = 0;
    unsigned char is_preprocessor_line = 0;
    unsigned char is_single_quoted = 0;
    unsigned char is_double_quoted = 0;
    unsigned char literal = 0;

    fillBuf();

    #ifdef _WIN32
    // Work around newline replacement on Windows;
    extern int fileno(FILE*);
    setmode(fileno(stdout), O_BINARY);
    #endif

    while (getNextByte()) {
        if (nthByte(0) == ':' && nthByte(1) == '\n') {
            printf(" {");
            open_braces++;
        }
         else {
            printf("%c", nthByte(0));
        }

        if (!is_preprocessor_line) {
            // Handle closing brace insertion;
            if (nthByte(0) == '\n' && !is_preprocessor_line) {
                while (open_braces && previous_indent > nextLineIndent()) {
                    for (unsigned char i = 4; i < previous_indent << 2; i++) {
                        printf(" ");
                    }

                    printf("}\n");
                    previous_indent--;
                    open_braces--;
                }

                previous_indent = nextLineIndent();
                #ifdef debug
                printf("// Next indent level: %d\n", previous_indent);
                #endif

            }
            // Handle semicolon insertions;
            if (nthByte(1) == '\n' &&
                nthByte(0) != ';' &&
                nthByte(0) != ',' &&
                nthByte(0) != '&' &&
                //                nthByte(0) != '+' &&
                //                nthByte(0) != '-' &&
                nthByte(0) != '*' &&
                nthByte(0) != '/' &&
                nthByte(0) != '>' &&
                nthByte(0) != '<' &&
                nthByte(0) != '"' &&
                nthByte(0) != '\'' &&
                nthByte(0) != '|' &&
                nthByte(0) != '{' &&
                nthByte(0) != '}' &&
                nthByte(0) != ' ' &&
                nthByte(0) != '\n' &&
                nthByte(0) != ':') {
                printf(";");
            }
        }
        
        if (nthByte(0) == '\n') {
            is_preprocessor_line = 0;
        }

        if (nthByte(0) == '#' && !(is_single_quoted || is_double_quoted)) {
            is_preprocessor_line = 1;
        }

        if (nthByte(0) == '\'' && !literal) {
            is_single_quoted = !is_single_quoted;
        }

        if (nthByte(0) == '\'' && !literal) {
            is_double_quoted = !is_double_quoted;
        }
        
        if (nthByte(0) == '\\') {
            literal = 1;
        }
        else {
            literal = 0;
        }
        
        line_pos +=1;
    }

    return 0;
}
