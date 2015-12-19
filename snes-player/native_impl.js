
var LibraryNativeImpl = {

	SaveState: function(fileName, state)
	{
		if(typeof externalSaveState !== "undefined")
			externalSaveState(fileName, state);
		else
			console.log("[WARNING]: externalSaveState() is not implemented by the host code. State save was ignored.");
	},
	
	GetCanvasWidth : function()
	{
		return window.innerWidth;
	},
	
	GetCanvasHeight : function()
	{
		return window.innerHeight;
	},
};

mergeInto(LibraryManager.library, LibraryNativeImpl);
 