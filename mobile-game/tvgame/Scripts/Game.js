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

var gameconfig={
	width:window.innerWidth,
	height:window.innerHeight,
	baseobjs:{},
};

var HttpVerb = 
{
	GET: "GET",
	POST: "POST",
	PUT: "PUT",
	DELETE: "DELETE"
};
var ConcurrentHttpRequests = 3;
var DefaultRequestTimeout = 10000;
var gameurl;
var funcstr="func";

var cmds={
	s:function(){
		var args = Array.prototype.slice.call(arguments);
		var s=args[0];
		for(var i=1;i<args.length;++i) {
			s+="_"+args[i];
		}
		return s;   
		},
	v:function(){
		var args = Array.prototype.slice.call(arguments);
		var o= args.shift();
		for(var i=0;i<args.length;++i) {
			o=o[args[i]];
		}
		return o;   
		},
	url:function(obj,idx){
		return url_base +"/" +v(obj,gameselect,"url")+"/"+v(obj,gameselect,idx);
		},
	fog:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift();
		var scene= args.shift();
		var fog=gameconfig.config.scene.fog;
		gameconfig.scene.fog = new THREE.Fog( fog[0], fog[1], fog[2] );
		},
	camera:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift();
		var scene= args.shift();
		var camera=gameconfig.config.scene.camera;
		loadobjs(null,camera,1,gameconfig.camera);
		loop=2;
		loadobjs({item:function(str){return s("camera",str);}},camera,2,gameconfig.camera);
		scene.add( gameconfig.camera );
		},
	PerspectiveCamera:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters=args.shift().PerspectiveCamera;
		var loop= args.shift();
		var camera= args.shift();
		if(loop!=1) return;
		gameconfig.camera = new THREE.PerspectiveCamera(parameters[0],gameconfig.width/ gameconfig.height,parameters[2],parameters[3]);
		},
	camera_position:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters=args.shift().position;
		var loop= args.shift();
		var camera= args.shift();
		if(camera) {
			if(loop!=2) return;
		}else{
			if(typeof loop === 'object') {
				camera=loop;
			}
			if(camera==null) return;
		}
		camera.position.set(parameters[0],parameters[1],parameters[2]);
		},
	camera_lookAt:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().lookAt;
		var loop= args.shift();
		var camera= args.shift();
		if(camera) {
			if(loop!=2) return;
		}else{
			if(typeof loop === 'object') {
				camera=loop;
			}
			if(camera==null) return;
		}
		camera.lookAt(new THREE.Vector3(parameters[0],parameters[1],parameters[2]));
		},
	AmbientLight:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift();
		var scene= args.shift();
		var light=gameconfig.config.scene.AmbientLight;
		scene.add( new THREE.AmbientLight( light ) );
		},
	DirectionalLight:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift();
		var scene= args.shift();
		var parameters=gameconfig.config.scene.DirectionalLight;
		var attrib=gameconfig.config.scene.DirectionalLight.attrib;
		var light = new THREE.DirectionalLight( attrib[0], attrib[1] );
		loadobjs({item:function(str){return s("DirectionalLight",str);}},parameters,light);
		scene.add( light );
		},
	DirectionalLight_position:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().position;
		var light= args.shift();
		light.position.set( parameters[0],  parameters[1],  parameters[2] );
		},
	DirectionalLight_castShadow:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().castShadow;
		var light= args.shift();
		light.castShadow = parameter;
		},
	DirectionalLight_shadowMapWidth:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().shadowMapWidth;
		var light= args.shift();
		light.shadowMapWidth = parameter;
		},
	DirectionalLight_shadowMapHeight:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().shadowMapHeight;
		var light= args.shift();
		light.shadowMapHeight = parameter;
		},
	DirectionalLight_shadowMapDarkness:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().shadowMapDarkness;
		var light= args.shift();
		light.shadowMapDarkness = parameter;
		},
	DirectionalLight_shadowCascade:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().shadowCascade;
		var light= args.shift();
		light.shadowCascade = parameter;
		},
	DirectionalLight_shadowCascadeCount:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().shadowCascadeCount;
		var light= args.shift();
		light.shadowCascadeCount = parameter;
		},
	DirectionalLight_shadowCascadeNearZ:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().shadowCascadeNearZ;
		var light= args.shift();
		light.shadowCascadeNearZ=[ parameters[0],  parameters[1],  parameters[2] ];
		},
	DirectionalLight_shadowCascadeFarZ:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().shadowCascadeFarZ;
		var light= args.shift();
		light.shadowCascadeFarZ=[ parameters[0],  parameters[1],  parameters[2] ];
		},
	DirectionalLight_shadowCascadeWidth:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().shadowCascadeWidth;
		var light= args.shift();
		light.shadowCascadeWidth=[ parameters[0],  parameters[1],  parameters[2] ];
		},
	DirectionalLight_shadowCascadeHeight:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().shadowCascadeHeight;
		var light= args.shift();
		light.shadowCascadeHeight=[ parameters[0],  parameters[1],  parameters[2] ];
		},
	renderer:function(){
		var parameters=gameconfig.config.scene.renderer;
		var renderer ;

		if(parameters.antialias) {
			renderer = new THREE.WebGLRenderer( { antialias: parameters.antialias } );
		}else{
			renderer = new THREE.WebGLRenderer();
		}
		gameconfig.renderer = renderer;
		renderer.setSize( gameconfig.width, gameconfig.height );
		loadobjs({item:function(str){return s("renderer",str);}},parameters,renderer);
		},
	renderer_setClearColor:function(){
			var args = Array.prototype.slice.call(arguments);
			var parameters= args.shift().setClearColor;
			var renderer= args.shift();
			if(parameters.rgb=="fog") {
				renderer.setClearColor( gameconfig.scene.fog.color, parameters.alpha );
			}else{
				renderer.setClearColor( parameters.rgb, parameters.alpha );
			}
		},
	renderer_gammaInput:function(){
			var args = Array.prototype.slice.call(arguments);
			var parameter= args.shift().gammaInput;
			var renderer= args.shift();
			renderer.gammaInput =parameter;
		},
	renderer_gammaOutput:function(){
			var args = Array.prototype.slice.call(arguments);
			var parameter= args.shift().gammaOutput;
			var renderer= args.shift();
			renderer.gammaOutput =parameter;
		},
	renderer_shadowMapEnabled:function(){
			var args = Array.prototype.slice.call(arguments);
			var parameter= args.shift().shadowMapEnabled;
			var renderer= args.shift();
			renderer.shadowMapEnabled =parameter;
		},
	renderer_shadowMapCascade:function(){
			var args = Array.prototype.slice.call(arguments);
			var parameter= args.shift().shadowMapCascade;
			var renderer= args.shift();
			renderer.shadowMapCascade =parameter;
		},
	renderer_shadowMapType:function(){
			var args = Array.prototype.slice.call(arguments);
			var parameter= args.shift().shadowMapType;
			var renderer= args.shift();
			renderer.shadowMapType =parameter;
		},
	mesh:function(){
		var output={};
		var args = Array.prototype.slice.call(arguments);
		var objs= args.shift();
		var scene= args.shift();
		var parameters=gameconfig.config.scene.mesh;
		loadobjs({item:function(str){return s("mesh",str);}},parameters,1,output);

		var mesh = new THREE.Mesh( output.geometry, output.material );
		scene.add( mesh );
		loadobjs({item:function(str){return s("mesh",str);}},parameters,2,mesh);
		},
    mesh_geometry:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters=args.shift().geometry;
		var loop= args.shift();
		var output= args.shift();
		if(loop!=1) return;
		loadobjs(null,parameters,output);
		},
	PlaneGeometry:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().PlaneGeometry;
		var output= args.shift();
		output.geometry = new THREE.PlaneGeometry( parameters[0],parameters[1] );
		},
	mesh_material:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters=args.shift().material;
		var loop= args.shift();
		var output= args.shift();
		if(loop!=1) return;
		loadobjs(null,parameters,output);
		},
	MeshPhongMaterial:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().MeshPhongMaterial;
		var output= args.shift();
		var outs={};
		loadobjs(null,parameters,outs);
		output.material=new THREE.MeshPhongMaterial( outs );
		},
	color:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().color;
		var outs= args.shift();
		outs.color=parameter;
		},
	map:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().map;
		var outs= args.shift();
		loadobjs({item:function(str){return s("map",str);}},parameters,outs);
		},
	map_url:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().url;
		var outs= args.shift();
		outs.map=THREE.ImageUtils.loadTexture( gameurl+"/"+parameter );
	},
	map_repeat:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters= args.shift().repeat;
		var outs= args.shift();
		outs.map.repeat.set( parameters[0], parameters[1] );
	},
	map_wrapS:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().wrapS;
		var outs= args.shift();
		outs.map.wrapS=parameter;
	},
	map_wrapT:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter= args.shift().wrapT;
		var outs= args.shift();
		outs.map.wrapT=parameter;
	},
	mesh_receiveShadow:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameter=args.shift().receiveShadow;
		var loop= args.shift();
		var mesh= args.shift();
		if(loop!=2) return;
		mesh.receiveShadow=parameter;
		},
	mesh_rotation:function(){
		var args = Array.prototype.slice.call(arguments);
		var parameters=args.shift().rotation;
		var loop= args.shift();
		var mesh= args.shift();
		if(loop!=2) return;
		mesh.rotation.x=parameters[0];
		mesh.rotation.y=parameters[1];
		mesh.rotation.z=parameters[2];
		},
	animation_camera:function(){
		var args = Array.prototype.slice.call(arguments);
		var camera= gameconfig.camera;
		var parameters= args.shift().camera;
		loadobjs({item:function(str){return s("camera",str);}},parameters,camera);
		},

	animation_objects:function(){
		var args = Array.prototype.slice.call(arguments);
		var objs;
		var parameters= args.shift().objects;
		gameconfig.objs=gameconfig.config.objects.objs;
		objs= gameconfig.objs;
		if(objs) {
			for(var i=0; i<objs.length;++i) {
				for(var j=0; j<objs[i].subs.length;++j) {
					for(var m=0; m<objs[i].subs[j].objs.length;++m) {
						if(objs[i].subs[j].objs[m].baseobj) {
							for(var k=0;k<objs[i].subs[j].objs[m].baseobj.children.length;++k) {
								objs[i].subs[j].objs[m].baseobj.children[k].morphTargetForcedOrder=[];
								for(var n=0;n<objs[i].subs[j].objs[m].baseobj.children[k].morphTargetInfluences.length;++n) {
									objs[i].subs[j].objs[m].baseobj.children[k].morphTargetInfluences[n]=0;
								}
								objs[i].subs[j].objs[m].baseobj.children[k].morphTargetForcedOrder.push(parameters[i]);
								objs[i].subs[j].objs[m].baseobj.children[k].morphTargetInfluences[parameters[i]]=1;
							}
					   }
					}
				}
			}
		}
		},
	baseobjs:function(){
		var baseurl=gameurl+"/"+gameconfig.config.objects.baseUrl+"/";
		var args = Array.prototype.slice.call(arguments);
		var parameters=args.shift();
		var config= args.shift();
		var objects= config.objects;
		objects.loadCounter=0;
		loadobjs({call:function(item,baseobjects,config){
				var objects= config.objects;
				var baseobjs=gameconfig.baseobjs;
				baseobjs[item]=(new Object3D(function(){call("loadobjs",config);})).load3DObjects(baseurl,objects.baseobjs[item]);
				if(config.objects.scale) {
					baseobjs[item].scale = config.objects.scale;
				}
				++objects.loadCounter;
			}
		},config.objects.baseobjs,config);
		},
	loadobjs:function(){
		var args = Array.prototype.slice.call(arguments);
		var config= args.shift();

		--config.objects.loadCounter;
		if(config.objects.loadCounter) return;
		loadobjs({singleitem:"objs"},config.objects,gameconfig.baseobjs);
		},
	objs:function(){
		var args = Array.prototype.slice.call(arguments);
		if(args[0].loadCounter) return;
		var objs= args.shift().objs;
		var baseobjs= args.shift();
		var scene=gameconfig.scene;
		for(var i=0;i<objs.length;++i) {
			var obj=objs[i];
			for(var j=0;j<obj.subs.length;++j) {
				var subobj=obj.subs[j];
				for(var m=0;m<subobj.objs.length;++m) {
					var baseobj=baseobjs[subobj.objs[m].obj];
					subobj.objs[m].baseobj=(new Object3D()).clone3DObjects(baseobj.children[0].geometry,baseobj.maps[subobj.objs[m].tex],baseobj.scale);
					subobj.objs[m].baseobj.position.x = obj.position[0];
					subobj.objs[m].baseobj.position.y = obj.position[1];
					subobj.objs[m].baseobj.position.z = obj.position[2];
					subobj.objs[m].baseobj.rotation.x = obj.rotation[0];
					subobj.objs[m].baseobj.rotation.y = obj.rotation[1];
					subobj.objs[m].baseobj.rotation.z = obj.rotation[2];
				}
				scene.add( subobj.objs[subobj.active].baseobj );
			}
	}
	},
};


var call=function() {
	var args = Array.prototype.slice.call(arguments);
	var c= cmds[args.shift()];
	if(c) {
		return c.apply(null,args);
	}
	return null;
}
var v=function (){
	var args =["v"];
	args = args.concat(Array.prototype.slice.call(arguments));
	return call.apply(null,args);
}
var s=function(){
	var args =["s"];
	args = args.concat(Array.prototype.slice.call(arguments));
	return call.apply(null,args);
}
var parsefunc=function()
{
	var args = Array.prototype.slice.call(arguments);
	var objs=args.shift();
	var key=args.shift();
	var item=args.shift();
	var func=v(cmds,key);

	if(item) {
		func=v(cmds,s(item,key));
	}
	if(func==null) {
		func=v(cmds,key);
	}
    if(func==null) return;

	objs[s(key,funcstr)]=func;
}
var url=function (obj,idx)
{
	return call("url",obj,idx);
}
var parseobjs=function(item, value)
{
	if (value && typeof value === 'object' ) {
		if(Object.prototype.toString.call(value) !== '[object Array]') {
			for(key in value){
				parsefunc(value,key,item);
			}
		}else{
			parsefunc(value,item);
		}
	}
	return value;
}
var loadobjs=function()
{
	var args = Array.prototype.slice.call(arguments);
	var cb= args.shift();
	var objs=args.shift();
	var nargs=[];
	for(item in objs) {
		if(cb && cb.singleitem) {
			item=cb.singleitem;
		}
		var call=v(objs,s(item,funcstr));
		if(objs[item]==null ||typeof objs[item]=== 'function') {
			continue;
		}
		if(cb && cb.item) {
			item=cb.item(item);
		}
		nargs=[];
		nargs.push(item);
		nargs.push(objs);
		nargs = nargs.concat(args);
		if(cb && cb.pre) {
			cb.pre.apply(null,nargs);
		}
		if(cb && cb.call) {
			cb.call.apply(null,nargs);
		}else{
			nargs.shift();
			if(call) {
				call.apply(null,nargs);
			}
		}
		if(cb && cb.suf) {
			cb.suf.apply(null,nargs);
		}
		if(cb && cb.singleitem) {
			break;
		}
	}

}
Object3D= function (onLoadComplete) {
	this.onLoadComplete = onLoadComplete;
	this.onLoadTextureComplete = function () {};
	this.root = new THREE.Object3D();
	var self=this;

	this.load3DObjects = function (url,loadobj){
		this.maps=loadTextures(url,loadobj.textures);
		var loader = new THREE.JSONLoader();

		loader.load( url + loadobj.obj, function( geometry ) {

			geometry.computeBoundingBox();
			self.root.position.y = - self.scale * geometry.boundingBox.min.y;

			var mesh = create3DObj( geometry, self.maps[ 0 ] );
			mesh.scale.set( self.scale, self.scale, self.scale );

			self.root.add( mesh );

			checkLoadingComplete();

		} );
		this.root.maps=this.maps;
		return this.root;
	}
	this.clone3DObjects = function ( geometry,map ,scale) {

		// BODY

		var mesh = create3DObj( geometry, map );
		mesh.scale.set( scale, scale, scale );

		this.root.add( mesh );
		return this.root;
	};

	var checkLoadingComplete=function () {

		self.onLoadComplete();

	};
	var checkLoadingTextureComplete=function () {

		self.onLoadTextureComplete();

	};

	var loadTextures=function ( baseUrl, textureUrls ) {

		var mapping = new THREE.UVMapping();
		var textures = [];

		for ( var i = 0; i < textureUrls.length; i ++ ) {

			textures[ i ] = THREE.ImageUtils.loadTexture( baseUrl + textureUrls[ i ], mapping, checkLoadingTextureComplete );
			textures[ i ].name = textureUrls[ i ];

		}

		return textures;

	};
	var create3DObj=function ( geometry, map ) {
		return new THREE.MorphBlendMesh( geometry, new THREE.MeshBasicMaterial( {map: map, morphTargets: true}) );
	};

}
