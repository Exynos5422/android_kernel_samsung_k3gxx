#ifndef _LIBFDT_ENV_H
#define _LIBFDT_ENV_H

<<<<<<< HEAD
=======
#include <linux/kernel.h>
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83
#include <linux/string.h>

#include <asm/byteorder.h>

#define fdt32_to_cpu(x) be32_to_cpu(x)
#define cpu_to_fdt32(x) cpu_to_be32(x)
#define fdt64_to_cpu(x) be64_to_cpu(x)
#define cpu_to_fdt64(x) cpu_to_be64(x)

#endif /* _LIBFDT_ENV_H */
