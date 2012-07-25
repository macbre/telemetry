LiteAjax('/json/daq/list', function(data) {
	for (i=0; i<data.length; i++) {
		file = data[i];
		
		tr = document.createElement('tr');
		$('list').appendChild(tr);

		url = '/daq/get/' + file[0];
		
		tr.innerHTML = '<td><a href="'+url+'">'+file[0]+'</a></td>' +
			// '<td>'+(new Date(file[1])).toLocaleString()+'</td>' +
			'<td>'+(file[2]>>1)+'</td>' +
			'<td><a style="cursor:pointer" onclick="daq_delete(\''+file[0]+'\', this.parentNode.parentNode)">Skasuj</a></td>';
	}
});

function daq_delete(name, row) {

	if (!confirm('Na pewno skasowaæ plik "'+name+'"?'))
		return;

	LiteAjax('/json/daq/delete/' + name, function(data) {
		if (data.result) {
			row.parentNode.removeChild(row);
		}
	});
}