#include <msp430.h>
#include "stateAs.h"
#include <p2switches.h>
int caseMethod(){
  switch (p2sw_read()){//
       case 1: 
           return 1;
       case 2:
           return 2;
       case 3:
           return 3;
       case 4:
           return 4;
   }
}
 
