// Copyright (c) 2020-2024 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// Edit E32Image header fields and compression type
//
//

#ifndef SRC_E32REBUILDER_H_
#define SRC_E32REBUILDER_H_

#include "task.hpp"

struct Args;
class E32Parser;

typedef std::vector<char> E32Buf;

class E32Rebuilder: public Task
{
	public:
		E32Rebuilder(Args* param);
		virtual ~E32Rebuilder();
		virtual void Run() final;
		void Compress(const E32Buf& e32File);
	private:
		void EditHeader();
		E32Buf ReCompress();
		void SaveAndValidate(const E32Buf& e32);
	private:
		E32Parser* iParser = nullptr;
		Args* iReBuildOptions = nullptr;
		char* iFile = nullptr;
		E32ImageHeader* iHdr = nullptr;
		std::streamsize iFileSize = 0;
};

#endif /* SRC_E32REBUILDER_H_ */
