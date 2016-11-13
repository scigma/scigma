extern "C" void auto_entry(int*, char*);

int main()
{
  int i(1);
  char c[9]="00000001";
  auto_entry(&i,c);
}
 
