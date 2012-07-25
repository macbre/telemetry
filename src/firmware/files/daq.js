var daq_progress;

// inicjalizacja
function daq_init() {

	LiteAjax('/json/status', function(d) {
		if (d['daq-samples'] > 0) {
			daq_progress();
			return;
		}

		var ch_html = '';
		
		for (i=0; i<new Number(d.ds); i++)
			ch_html += '<option value="ch-'+i+'">Czujnik #' + (i+1) + '</option>'
		
		$h('ch', ch_html);
})};

// start
function daq_start() {

	$('submit').disabled = true;

	query = '/json/daq/start/' + $('name').value.replace(/[^a-z0-9]/gi, '').toLowerCase() + 
					'/' + $('ch').options[$('ch').selectedIndex].value.split('-')[1] +
					'/' + parseInt($('delay').value) +
					'/' + parseInt($('samples').value);
	
	LiteAjax(query, function(d) {
		if (d.result == 0) {
			$c('daq', 'error');
			$('submit').disabled = false;
		}
		else {
			daq_progress();
		}
	});
	
	return false;
}

// informacja o trwaj¹cym pomiarze
function daq_progress() {
		$h('daq', 'Trwa akwizycja');
		$c('daq', 'progress');

		daq_progress = setInterval('daq_check()', 3000);
}

function daq_check() {
	LiteAjax('/json/status', function(d) {
		if (d['daq-samples'] == 0) {
			$h('daq', 'Akwizycja zakoñczona!');
			$('daq').style.background = 'none';
			clearTimeout(daq_progress);
		}
	});
}

daq_init();