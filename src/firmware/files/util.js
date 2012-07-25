function $(id) {return document.getElementById(id)}
function $h(id,v) {$(id).innerHTML = v}
function $c(id,v) {$(id).className = v}

function LiteAjax(url, handler) {
  var tr;
  obj = this;
  this.h = handler;

  if (window.XMLHttpRequest)
		tr =  new XMLHttpRequest();
  else if (window.ActiveXObject)
		tr = new ActiveXObject("Microsoft.XMLHTTP");
	else
		return;
		
  tr.onreadystatechange = function() {
		if (tr.readyState == 4 && tr.status == 200 && obj)
			obj.h(eval('('+tr.responseText+')'));
	};
	
  tr.open('POST', url);
  tr.setRequestHeader('X-Requested-By', 'AjaxLite');
  tr.send('');
}