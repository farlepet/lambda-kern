#ifndef LAMBDA_CONFIG_DEFS_H
#define LAMBDA_CONFIG_DEFS_H

/* Strictness levels */
#define LAMBDA_STRICTNESS_NONE       (  0) /* Do not enable any strict checks */
#define LAMBDA_STRICTNESS_USER       (  1) /* Strict checks where userland input is processed */
#define LAMBDA_STRICTNESS_NOIMPACT   (  2) /* Strict checks where performance impact is negligible */
#define LAMBDA_STRICTNESS_LOWIMPACT  (  3) /* Strict checks where performance impact is low */
#define LAMBDA_STRICTNESS_MEDIMPACT  (  4) /* Strict checks where performance impact is moderate */
#define LAMBDA_STRICTNESS_HIGHIMPACT (  5) /* Strict checks where performance impact is high */
#define LAMBDA_STRICTNESS_ALL        (255) /* Enable all strictness checks */

#define CHECK_STRICTNESS(X) ((X) <= CONFIG_STRICTNESS)

#endif
