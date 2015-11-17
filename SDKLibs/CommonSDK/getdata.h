/*
 * getdata.h
 *
 *  Created on: Nov 16, 2015
 *      Author: kolban
 */

#ifndef GETDATA_H_
#define GETDATA_H_

#include "espconn.h"
void getData(struct espconn *pEspconn, void (*callback)(char *data, uint16 length));

#endif /* GETDATA_H_ */
