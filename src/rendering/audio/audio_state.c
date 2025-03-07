#include "audio_state.h"

AudioState audio_state = {0};
AudioStream stream = {0};
thrd_t audio_thread = nullptr;