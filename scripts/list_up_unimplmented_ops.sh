#! /bin/bash -eu

cd $(git rev-parse --show-toplevel)
ALL_OPS=$(mktemp)
MADE_OPS=$(mktemp)
cat src/opcode_table.cc | grep '"[A-Z][A-Z][A-Z]*' | tr '"' ' ' | awk -e '{print $2}' | sort > $ALL_OPS
cat src/raijit.cc | grep "case [A-Z][A-Z][A-Z]*" | tr ":" " " | awk -e '{print $2}' | sort > $MADE_OPS
diff $ALL_OPS $MADE_OPS | grep "^<"
diff $ALL_OPS $MADE_OPS | grep "^<" | wc -l
