/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <thunder/audio_node.h>

void thunderAudioNodeInit(ThunderAudioNode* self, ThunderAudioNodeOutputFunc func, void* other_self)
{
    self->output = func;
    CLOG_ASSERT(other_self != 0, "Must provide valid self")
    self->_self = other_self;
}
