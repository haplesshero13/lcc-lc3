main(){
	f5();
}

f(i){i=f()+f();}

f2(i){i=f()+(i?f():1);}

f3(int i,int *p){register r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0,r8=0,r9=0,r10=0;*p++=i?f():0;}

int aa[10],ab[10];
int i;
f4(){register r6=0,r7=0,r8=0,r9=0,r10=0,r11=0;i=aa[i]+ab[i] && i && aa[i]-ab[i];}
/* f4 causes parent to spill child on vax when odd double regs are enabled */

int j=1, k=2, m=3, n=3;
//int *A, *B, x;
int A[10] = {1,1,2,3,4,5,6,7,8,9};
int B[10] = {1,1,2,3,4,5,6,7,8,9};
int x=0;

f5(){
	x=A[k*m]*A[j*m]+B[k*n]*B[j*n];
	printf("%d\n",x);
	x=A[k*m]*B[j*n]-B[k*n]*A[j*m];
	printf("%d\n",x);
}  
