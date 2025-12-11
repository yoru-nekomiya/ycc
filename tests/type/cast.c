int main(){
   assert(131585, (int)8590066177, "(int)8590066177");
   assert(513, (short)8590066177, "(short)8590066177");
   assert(1, (char)8590066177, "(char)8590066177");
   assert(1, (long)1, "(long)1");
   assert(0, (long)&*(int *)0, "(long)&*(int *)0");

   assert(-1, (int)(char)-1, "(int)(char)-1");
   assert(-1, (long)(char)-1, "(long)(char)-1");
   assert(-1, (long)(short)-1, "(long)(short)-1");
   assert(-1, (long)-1, "(long)-1");

   assert(-128, (char)128, "(char)128");

   int x=5; long y=(long)&x;
   assert(5, *(int*)y, "int x=5; long y=(long)&x; *(int*)y;");
  
  return 0;
}
