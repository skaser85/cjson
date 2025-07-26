#include <stdio.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

void add_comma(String_Builder *sb) {
    da_append(sb, ',');
}

void strip_trailing_comma(String_Builder *sb) {
    char last = sb->items[sb->count-1];
    if (last == ',') {
        sb->count--;
    }
}

void begin_object(String_Builder *sb) {
    da_append(sb, '{');
}

void end_object(String_Builder *sb) {
    strip_trailing_comma(sb); 
    da_append(sb, '}');
    add_comma(sb);
}

void begin_array(String_Builder *sb) {
    da_append(sb, '[');
}

void end_array(String_Builder *sb) {
    strip_trailing_comma(sb); 
    da_append(sb, ']');
    add_comma(sb);
}

void add_key(String_Builder *sb, const char *key) {
    sb_appendf(sb, "\"%s\": ", key);
}

void add_string(String_Builder *sb, const char *string) {
    sb_appendf(sb, "\"%s\"", string);
    add_comma(sb);
}

void add_float(String_Builder *sb, float value) {
    sb_appendf(sb, "%f", value);
    add_comma(sb);
}

void add_bool(String_Builder *sb, bool boolean) {
    boolean ? sb_appendf(sb, "true") : sb_appendf(sb, "false");
    add_comma(sb);
}

void add_null(String_Builder *sb) {
    sb_appendf(sb, "null");
    add_comma(sb);
}

int main() {

    String_Builder sb = {0};

    begin_object(&sb);
        add_key(&sb, "first");
        add_string(&sb, "item1");
        add_key(&sb, "second");
        add_float(&sb, 3.1425f);
        add_key(&sb, "list");
        begin_array(&sb);
            add_string(&sb, "a");
            add_string(&sb, "b");
            add_string(&sb, "c");
            add_float(&sb, 1234);
            add_null(&sb);
            add_bool(&sb, true);
            add_bool(&sb, false);
        end_array(&sb);
        add_key(&sb, "third");
        add_bool(&sb, true);
        add_key(&sb, "fourth");
        begin_object(&sb);
            add_key(&sb, "inner1");
            add_string(&sb, "heyo");
            add_key(&sb, "inner2");
            begin_array(&sb);
                add_string(&sb, "whoa");
                add_string(&sb, "my");
                add_string(&sb, "dude");
            end_array(&sb);
        end_object(&sb);
        add_key(&sb, "fifth");
        add_string(&sb, "the end");
    end_object(&sb);
    strip_trailing_comma(&sb);

    String_View sv = sb_to_sv(sb);
    nob_log(NOB_INFO, SV_Fmt, SV_Arg(sv));

    return 0;
}
