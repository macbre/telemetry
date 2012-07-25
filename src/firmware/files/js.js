function updateTemp() {
	te = $('temperatures');

	if (!te) return;

	te.className = 'progress';
	LiteAjax('/json/ds', function(data) {
		t = te.getElementsByTagName('td');
		
		for (i=0; i<data.length; i++) {
			t[i].innerHTML = data[i].toFixed(1);
			t[i].className = 'temp_' + Math.floor(data[i]/10) + '0';
		}

		te.className = '';
		
		setTimeout(updateTemp, 1000);
	});
}

function setTime(obj) {

	obj.innerHTML = 'Czekaj...';
	obj.className = 'progress';

	t = new Date();

	offset    = (t.getTimezoneOffset() / 60).toFixed();
	timestamp = (t.getTime()/ 1000).toFixed() - (offset * 3600);

	LiteAjax('/json/timer/' + timestamp + '_' + offset, function(data) {

		obj.className = '';
		obj.innerHTML = 'Czas zaktualizowany ('+t.toLocaleString()+')';

	});
}