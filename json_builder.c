#include <stdio.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "./nob.h"

void begin_object(String_Builder *sb) {
    da_append(sb, '{');
}

void end_object(String_Builder *sb) {
    da_append(sb, '}');
}

void begin_array(String_Builder *sb) {
    da_append(sb, '[');
}

void end_array(String_Builder *sb) {
    da_append(sb, ']');
}

void add_key(String_Builder *sb, const char *key) {
    sb_appendf(sb, "\"%s\": ", key);
}

void add_string(String_Builder *sb, const char *string) {
    sb_appendf(sb, "\"%s\"", string);
}

void add_float(String_Builder *sb, float value) {
    sb_appendf(sb, "%f", value);
}

void add_bool(String_Builder *sb, bool boolean) {
    boolean ? sb_appendf(sb, "true") : sb_appendf(sb, "false");
}

void add_null(String_Builder *sb) {
    sb_appendf(sb, "null");
}

void add_member_string(String_Builder *sb, const char *key, const char *value) {
    add_key(sb, key);
    add_string(sb, value);
}

void add_member_float(String_Builder *sb, const char *key, float value) {
    add_key(sb, key);
    add_float(sb, value);
}

void add_member_bool(String_Builder *sb, const char *key, bool boolean) {
    add_key(sb, key);
    add_bool(sb, boolean);
}

void add_member_null(String_Builder *sb, const char *key) {
    add_key(sb, key);
    add_null(sb);
}

void add_member_object(String_Builder *sb, const char *key) {
    add_key(sb, key);
    begin_object(sb);
}

void add_member_array(String_Builder *sb, const char *key) {
    add_key(sb, key);
    begin_array(sb);
}

void add_comma(String_Builder *sb) {
    da_append(sb, ',');
}

int main() {

    String_Builder sb = {0};

    begin_object(&sb);
        add_member_string(&sb, "first", "value1");
        add_comma(&sb);
        add_member_string(&sb, "second", "another value");
        add_comma(&sb);
        add_member_array(&sb, "list");
            add_string(&sb, "item1");
            add_comma(&sb);
            add_string(&sb, "item2");
            add_comma(&sb);
            add_float(&sb, 3.14f);
        end_array(&sb);
        add_comma(&sb);
        add_member_bool(&sb, "cool", false);
        add_comma(&sb);
        add_member_string(&sb, "end", "the end");
    end_object(&sb);

    String_View sv = sb_to_sv(sb);
    nob_log(NOB_INFO, SV_Fmt, SV_Arg(sv));

    return 0;
}
