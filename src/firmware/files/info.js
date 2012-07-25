function getStatus(h) {LiteAjax('/json/status', h);}

function updateInfo() {LiteAjax('/json/status', function(d) {

	$h('rx-tx', 'Rx: '+d.rx+' / Tx: ' + d.tx);
	$h('ds',d.ds);

	if (d.card) $h('card',d.card + ' (' + d['card-size'] + ' kB)');
	else $h('card','<em>brak</em>');

	$h('eeprom',d.eeprom + ' kB');

	u = d.uptime; mf = Math.floor;
	$h('uptime',mf(u/3600 % 24) + 'h ' + mf(u/60 % 60) + 'm ' + mf(u%60) + 's');

	//setTimeout('updateInfo()', 1000);
});}