--[[
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
--]]
dorequest =function (socket,cmd)
    local cmd=string.gsub(cmd, "%%20", " ")
	local msg=""
	local file=io.popen(cmd,"r")
	for line in file:lines() do 
	  msg=string.format("%s %s\n",msg,line) 
	end
	file:close ()
	wcap_sendmsg(socket,nil,nil,msg)
end