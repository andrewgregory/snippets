#include "tap/tap.c"
#include "../fuzzy.c"

int main(void) {
    tap_plan(12);

    tap_is_str(strcasechr("aBcDeF", 'd'), "DeF", NULL);
    tap_is_str(strcasechr("aBcDeF", 'D'), "DeF", NULL);
    tap_is_str(strcasechr("aBcDeF", 'e'), "eF", NULL);
    tap_is_str(strcasechr("aBcDeF", 'E'), "eF", NULL);

    tap_is_str(strfuzzy("abcdefg", "adg"), "g", NULL);
    tap_is_str(strfuzzy("abcdefg", "cdg"), "g", NULL);
    tap_is_str(strfuzzy("abcdefg", "ace"), "efg", NULL);
    tap_is_str(strfuzzy("abcdefg", "cb"), NULL, NULL);

    tap_is_str(strcasefuzzy("abcdefg", "aDg"), "g", NULL);
    tap_is_str(strcasefuzzy("abcdefg", "cDg"), "g", NULL);
    tap_is_str(strcasefuzzy("abcdefg", "aCe"), "efg", NULL);
    tap_is_str(strcasefuzzy("abcdefg", "cB"), NULL, NULL);

    return tap_finish();
}
