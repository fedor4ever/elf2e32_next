/*
 * E32Rebulder.h
 *
 *  Created on: 6 янв. 2020 г.
 *      Author: Artiom
 */

#ifndef SRC_E32REBUILDER_H_
#define SRC_E32REBUILDER_H_

#include "task.hpp"

struct Args;

class E32Rebuilder: public Task
{
	public:
		E32Rebuilder(Args* param);
		virtual ~E32Rebuilder();
		virtual void Run() final;
	private:
		Args* iReBuildOptions = nullptr;
};

#endif /* SRC_E32REBUILDER_H_ */
