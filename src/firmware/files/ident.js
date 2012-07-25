ident_channel=0;
ident_channels=5;
ident_time=30000;

function ident_init() {
	$h('ident', 'Inicjalizacja procedury identyfikacyjnej');
	$('ident').className = 'progress';

	ident_channel = 0;

	setTimeout('ident_channel_on()', 1000);
}

function ident_channel_on() {

	if (ident_channel > ident_channels) {
		$h('ident', 'Procedura zakoñczona');
		$('ident').className = '';
		return;
	}

	$h('ident', 'Kana³ #' + (ident_channel+1) + ': za³¹cz');
	$('ident').className = 'progress on';

	LiteAjax('/json/pwm/set/'+ident_channel+'/255', function() {});

	setTimeout('ident_channel_off()', ident_time);
}

function ident_channel_off() {
	$h('ident', 'Kana³ #' + (ident_channel+1) + ': wy³¹cz');
	$('ident').className = 'progress off';

	LiteAjax('/json/pwm/set/'+ident_channel+'/0', function() {});

	ident_channel++;

	setTimeout('ident_channel_on()', ident_time * 5);
}