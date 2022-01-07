#!/bin/bash
${MIPS_TOOLCHAIN_DIR}/${MIPS_PREFIX}objdump -d  $1 | cut -f 3 | sed "s/^move$/addu/g;s/^nop$/sll/g;s/^li$/ori/g" | sort |  uniq | grep "^[a-z]*$" | xargs
