/*
 * Copyright 2016 Joey  <joeyye@foxmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "data.h"

vaglContext ctx;
int main(int argc, char *argv[])
{
	if(argc<2){
		printf("Usage: %s file\n",argv[0]);
		return 1;
	}
	openavfile(&ctx,argv[1]);
	for (;;){
		if(decode_frame(&ctx)==AVERROR_EOF)
			break;
    }
	closeavfile(&ctx);
	return 0;
}
