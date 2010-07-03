#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include "tts/tts.h"


extern android_tts_engine_t *android_getTtsEngine();

int main()
{
  android_tts_engine_t* e = android_getTtsEngine();
  e->funcs->init(0,0,"");
  return 0;
}
