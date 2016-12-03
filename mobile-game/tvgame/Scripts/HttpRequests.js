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
var HttpRequests = new function()
{
	var _Requests = [];
	var _CurrentRequests = [];

	var _MakeRequests = function()
	{
		for(var i = _CurrentRequests.length; i < ConcurrentHttpRequests && i < _Requests.length; i++)
        {
            var r = _Requests.splice(0,1)[0];
            var xhr = new XMLHttpRequest();
            r.xhr = xhr;

            var timeout = r.timeout || DefaultRequestTimeout;

            xhr.onreadystatechange = function(req)
            {
                return function()
                {
                    clearTimeout(req._timeout);
                    if(this.readyState == this.DONE)
                    {
                        if(this.status == 0) // also means CORS failed.
                        {
                            this.NetworkFailed = true;
                        }
                        if(req.callback)
                        {
                            var cb = req.callback;
                            req.callback = null;
                            cb(req, this);
                        }
                        _MakeRequests();
                    }   
                };
            }(r);

            var url;
            var s = r.service;

            if(s)
            {
                url = s.Location + r.url;
            }
            else
            {
                url = r.url;
            }

            xhr.open(r.verb, url, true);
			xhr.setRequestHeader("If-None-Match","some-random-string");
			xhr.setRequestHeader("Cache-Control","no-cache,max-age=0");
			xhr.setRequestHeader("Pragma","no-cache");
            if(r.responseType)
            {
                xhr.responseType = r.responseType; 
            }

            if(r.headers)
            {
            	var h = r.headers;
                for(var i in h)
                {
                    xhr.setRequestHeader(i, h[i]);
                }
            }
            if(s)
            {
                // Need LayoutTemplates to work with im-wskid, im-secret
                if(s.Type != ServiceType.LayoutTemplates)
                {
                    if(s.WskId)
                    {
                        xhr.setRequestHeader("im-wskid", s.WskId);
                    }
                    if(s.SecretToken != undefined)
                    {
                        xhr.setRequestHeader("im-secret", s.SecretToken);
                    }
                }
            }
            if(r.data)
            {
                xhr.send(r.data);
            }
            else
            {
                xhr.send();
            }
            r._timeout = setTimeout(function(req)
            {
                return function()
                {
                    if(req.callback)
                    {
                        console.log("Request has timed out", req.url, req.service);
                        var xhr = req.xhr;
                        if(xhr)
                        {
                            xhr.abort();
                        }
                        var cb = req.callback;
                        req.callback = null;
                        if(cb)
                        {
                            cb(req, req.xhr);
                        }
                    }
                    _MakeRequests();
                };
            }(r), timeout);
        }
	};

	this.Make = function(req)
	{
		if(!req.priority)
        {
            req.priority = 0;
        }

        _Requests.push(req);
        _Requests.sort(function(a,b) 
        {
            return b.priority - a.priority;
        });

        _MakeRequests();

        return req;
	};
}();