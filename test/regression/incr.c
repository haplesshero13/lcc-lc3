char gx = 'd';
char gy = 'd';
int a = 60;
int b = 10;

main() {
	memchar();
	memint();
	regchar();
	regint();
}

memchar() {
	char x, *p;

	&x, &p;
	x = gx;
	p = &gy;
	x = *p++;
	x = *++p;
	x = *p--;
	x = *--p;
	printf("%c %c\n",x,*p);
}

memint() {
	int x, *p;

	&x, &p;

	x = a;
	p = &b;
	x = *p++;
	x = *++p;
	x = *p--;
	x = *--p;
	printf("%d %x %o %b %d\n",x, x, x, x,*p);
	printf("%d %d\n", x,*p);
}

regchar() {
	register char x, *p;
	x = gx;
	p = &gy;

	x = *p++;
	x = *++p;
	x = *p--;
	x = *--p;
	printf("%c %c\n",x,*p);
}

regint() {
	register int x, *p;

	x = a;
	p = &b;
	x = *p++;
	x = *++p;
	x = *p--;
	x = *--p;
	printf("%x %d\n",x,*p);
}
