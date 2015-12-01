
int globalVar=0;
int check(int x,int y)
{
	return 1;
}
void do_math(int *x,char t) {
    *x += 5;
    if(check(1,2))			/*test case for function check*/
		globalVar=0;
	if (int x = check(1,3)) {
   x=0;
 }
}

void iffuncCheck()
{
	iffuncCheck();
	if(2==1)
	{
		int k=1;
	}
	else {
		int y=2;
	}
	int i=0;
	for(i=0;int xyz=0&&i<10;i++)
	{
		i++;
	}
	while(int x=3)
	{
		i++;
	}
	while(i!=3)
	{
		i--;
	}
}

void testSwitchCase()
{
	
	int x=3;
	switch(x)
	{
		case 3:x=2; break;
		case 2: x=3;break;
		case 1:x=4;break;
	}
	char ch='a';
	switch(ch)
	{
		case 'a': ch='b';break;
		case 'b':ch='c';break;
	}
}

int main(void) {
    int result = -1, val = 4;
    do_math(&val,'a');
    result=5;
    return result;
}


