/*
 * startListening.h
 *
 *  Created on: Nov 16, 2015
 *      Author: kolban
 */

#ifndef STARTLISTENING_H_
#define STARTLISTENING_H_

#include "espconn.h"
void startListening(uint16 port, void (*callback)(struct espconn *pEspconn));

#endif /* STARTLISTENING_H_ */
