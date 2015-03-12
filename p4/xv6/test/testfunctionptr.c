#include <stdio.h>

void testfunction() {

  return;


}


int main() {

  int a=0;
  a=1;


  printf("\n The address to main() is %p", main);
  printf("\n The address to testfunction() is %p", testfunction);
  return 0;

}
