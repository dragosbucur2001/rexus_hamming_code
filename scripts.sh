gcc main.c -g && ./a.out | sort | uniq -c
python3 decoder.py | rg "Good Message" |  wc -l
