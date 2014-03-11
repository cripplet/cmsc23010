#ifndef HASHPACKETTEST_H_
#define HASHPACKETTEST_H_

#include "../result.h"

result *serialHashPacketTest(int numMilliseconds,
							float fractionAdd,
							float fractionRemove,
							float hitRate,
							int maxBucketSize,
							long mean,
							int initSize);


result *parallelHashPacketTest(int numMilliseconds,
                                                    float fractionAdd,
                                                        float fractionRemove,
                                                        float hitRate,
                                                        int maxBucketSize,
                                                        long mean,
                                                        int initSize,
                                                        int numWorkers,
                                                        int log_threads,
                                                        int M,
                                                        int H,
                                                        int is_dropped);



#endif /* HASHPACKETTEST_H_ */
