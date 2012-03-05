#ifndef DARWIN_CRASH_H
#define DARWIN_CRASH_H

extern char ReasonForCrash[10000];

class FatalCrashException
{
public:
	int reason;
};

#endif /* DARWIN_CRASH_H */
