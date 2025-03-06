#include "audio_state.h"

AudioState audio_state = {nullptr};
AudioStream stream = {nullptr};
thrd_t audio_thread = nullptr;