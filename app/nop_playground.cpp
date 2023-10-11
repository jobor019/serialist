

#include "core/node_base.h"
#include "core/allocator.h"



int main() {
    Voices<PartialNote> voices = Voices<PartialNote>::empty_like();
    voices = Voices<PartialNote>::zeros(4);
}