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
function rtrm(str)
	x=string.find(str," ")
	if x==nil then
	   return str
	end
	return string.sub(str,1,x-1)
end
function decodeurl(address)
	local str_start,str_end=string.find(address, '?')
	local file=address

	if str_start then
		file=string.sub (address,1,str_start-1)
	end
	file=rtrm(file)
	if file=="/" then
	   file="index.html"
	   local fp = io.open(file, "r");
	   if fp==nil then
		  file="scripts/index.lua"
	   else
	      fp:close()
	   end
	else
	  file=string.sub (file,2,string.len(file))
	end
	
	if str_start==nil then
		return file,nil,nil
	end
	

	local request=string.sub(address, str_end+1,string.len(address))
	local cmd;
	local cmd1;
	local x,x1=string.find(request,"tapx=");
	local y,y1=string.find(request,"tapy=");
	if x and y then
	  local xval=string.sub (request,x1+1,y-2)
	  local yval=string.sub (request,y1+1,string.len(request))
	  cmd=string.format("input tap %s %s",xval,yval)
	end

	local cmdi,cmdi1=string.find(request,"command=")
	if cmdi then
		cmd1=string.sub(request,cmdi1+1,string.len(request))
	end
	
	local key,key1=string.find(request,"keyevent=")
	if key then
		local keyval=string.sub (request,key1+1,string.len(request))
	    cmd=string.format("input keyevent %s",keyval)
	end
	
	local swipe,swipe1=string.find(request,"swipe=");
	if swipe then
		local location,location1=string.find(request,"location");
		if location then
			x1,x1e=string.find(request,"&x1=");
			y1,y1e=string.find(request,"&y1=");
			x2,x2e=string.find(request,"&x2=");
			y2,y2e=string.find(request,"&y2=");
			if x1 and y1 and x2 and y2 then
				x1v=string.sub (request,x1e+1,y1-1)
				y1v=string.sub (request,y1e+1,x2-1)
				x2v=string.sub (request,x2e+1,y2-1)
				y2v=string.sub (request,y2e+1,string.len(request))
				cmd=string.format("input swipe %s %s %s %s ",x1v,y1v,x2v,y2v)
			end
		end
		
	end
	
	return file,cmd,cmd1
end	


analyze = function(socket, buffer)
	local str1_start, str1_end=string.find(buffer, ' HTTP')
	local address
	local type=1
	if str1_start==nil then 
		return -2;
	end
	local str_start,str_end=string.find(buffer, 'GET ')
	if str_start then
	   address=string.sub (buffer,str_end+1,str1_start)
	   type=1
	end
	str_start,str_end=string.find(buffer, 'POST ')
	if  str_start then
	   address=string.sub (buffer,str_end+1,str1_start)
	   type=2
	end
	local file,cmd,cmd1=decodeurl(address)
	if cmd then
		os.execute(cmd)
	end
	if file==nil then
	   return -3
	end
	if type==2 then
	   wcap_sendmsg(socket,"OK","POST","post successfully");
	end
	len=string.len(file)
	local ext=string.sub(file,len-3,len)
	if string.find(ext,"lua") then
		require(string.sub(file,1,len-4))
		dorequest(socket,cmd1)
	else
	   wcap_sendfile(socket,".",file)
	end
	return 0
end
 
