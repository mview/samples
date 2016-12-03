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

var path;
var url_base;
var gameindex="game.json";
var gameselect;
var intervaltime=20;
var lockchannel=false;

function start(){
	var req=GetRequest();
	if(req.game) {
		gameselect=req.game;
	}
	if(req.index) {
		gameindex=req.index;
	}
	if(req.lockchannel) {
		lockchannel=req.lockchannel;
	}
	path=location.pathname+"../";
	url_base="http://"+location.hostname+path; 
	
	HttpRequests.Make(
	{
		url: url_base + "/"+gameindex,
		verb: HttpVerb.GET,
		priority: 5,

		callback: function(request, response)
		{
			try
			{
				var obj = JSON.parse(response.responseText);
			}
			catch(err)
			{
				console.log( "Failed to load " + request.url);
				return;
			}

			if(!obj)
			{
				console.log("Bad response from " + request.url);
				return;
			}
			if(gameselect==null) {
				gameselect=obj.index;
			}
			gameurl=url_base+obj[gameselect].url;
			if(gameselect) {
				HttpRequests.Make(
				{
					url: url(obj,"config"),
					verb: HttpVerb.GET,
					priority: 5,

					callback: function(request, response)
					{
						try
						{
							var obj = JSON.parse(response.responseText,parseobjs);
						}
						catch(err)
						{
							console.log( "Failed to load " + request.url);
							return;
						}

						if(!obj)
						{
							console.log("Bad response from " + request.url);
							return;
						}
						loadgame(obj);
					}
				});
				HttpRequests.Make(
				{
					url: url(obj,"control"),
					verb: HttpVerb.GET,
					priority: 5,

					callback: function(request, response)
					{
						try
						{
							var obj = JSON.parse(response.responseText,parseobjs);
						}
						catch(err)
						{
							console.log( "Failed to load " + request.url);
							return;
						}

						if(!obj)
						{
							console.log("Bad response from " + request.url);
							return;
						}
						gameconfig.control=obj;
						gameconfig.control.url=request.url;
						setInterval(function(){
							HttpRequests.Make(
							{
								url: gameconfig.control.url,
								verb: HttpVerb.GET,
								priority: 5,

								callback: function(request, response)
								{
									try
									{
										var obj = JSON.parse(response.responseText,parseobjs);
									}
									catch(err)
									{
										console.log( "Failed to load " + request.url);
										return;
									}

									if(!obj)
									{
										console.log("Bad response from " + request.url);
										return;
									}
									if(gameconfig.config.remote.ignoreid==true || obj.id!=gameconfig.control.id) {
										obj.url=gameconfig.control.url;
										gameconfig.control=obj;
									}
								}
							});
							},intervaltime);
					}
				});
			}
		}
	});
}
function GetRequest()  
{  
	var url = location.search;   
	var theRequest = new Object();  
	if(url.indexOf("?") != -1)  
	{  
	  var str = url.substr(1);  
		strs = str.split("&");  
	  for(var i = 0; i < strs.length; i ++)  
		{  
		 theRequest[strs[i].split("=")[0]]=unescape(strs[i].split("=")[1]);  
		}  
	}  
	return theRequest;  
}  

function loadgame(obj) {
	gameconfig.config=obj;
	if ( ! Detector.webgl ) Detector.addGetWebGLMessage();

	init();
	animate();
}
function init() {

	gameconfig.container = document.createElement( 'div' );
	document.body.appendChild( gameconfig.container );

	gameconfig.scene = new THREE.Scene();
	loadobjs(null,gameconfig.config.scene,gameconfig.scene);

	gameconfig.container.appendChild( gameconfig.renderer.domElement );

	// STATS

	if(gameconfig.config && gameconfig.config.remote.showfps==true) {
		stats = new Stats();
		gameconfig.container.appendChild( stats.domElement );
	}

	// EVENTS

	window.addEventListener( 'resize', onWindowResize, false );

	loadobjs(null,gameconfig.config.objects,gameconfig.config);
}

// EVENT HANDLERS

function onWindowResize( event ) {

	gameconfig.width = window.innerWidth;
	gameconfig.height = window.innerHeight;

	gameconfig.renderer.setSize( gameconfig.width, gameconfig.height );

	gameconfig.camera.aspect = gameconfig.width/ gameconfig.height;
	gameconfig.camera.updateProjectionMatrix();

}

function animate() {

	requestAnimationFrame( animate );
	render();

	if(stats)stats.update();

}

function render() {

	if(gameconfig.control){ 
		if(!lockchannel&& gameconfig.control.global.game!=gameselect) {
			location.replace(location.origin + location.pathname +"?game="+gameconfig.control.global.game);
		}
		if( gameconfig.config && gameconfig.config.remote.control==true) {
			loadobjs({item:function(str){return s("animation",str);}},gameconfig.control.animation);
		}
	}
	gameconfig.renderer.render( gameconfig.scene, gameconfig.camera );

}


