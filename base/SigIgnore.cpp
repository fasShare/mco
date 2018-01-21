#include <SigIgnore.h>
#include <signal.h>

using moxie::SigIgnore;

SigIgnore::SigIgnore() {
    ::signal(SIGPIPE, SIG_IGN);
}

SigIgnore SigIgn;
