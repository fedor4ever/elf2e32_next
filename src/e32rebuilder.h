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
class E32Parser;

class E32Rebuilder: public Task
{
	public:
		E32Rebuilder(Args* param);
		virtual ~E32Rebuilder();
		virtual void Run() final;
	private:
		void EditHeader();
		void ReCompress();
	private:
		E32Parser* iParser = nullptr;
		Args* iReBuildOptions = nullptr;
		const char* iFile = nullptr;
		int iFileSize = 0;
};

#endif /* SRC_E32REBUILDER_H_ */
