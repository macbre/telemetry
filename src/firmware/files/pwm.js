var pwm_channels = 0;

function update_pwm() {LiteAjax('/json/pwm', function(d) {
	for (i=0; i<d.length; i++) {$h('pwm-'+i, d[i]); $('pwm-'+i+'-bar').style.width = d[i] + 'px'}
});}

function set_pwm(field) {
	val = new Number(field.value) % 256;
	ch = new Number(field.id.split('-')[1]);

	field.disabled = true;

	LiteAjax('/json/pwm/set/'+ch+'/'+val, function(d) {
		field.disabled = false;
	});

	return false;
}


function init_pwm(table) {LiteAjax('/json/pwm', function(d) {
	for (i=0; i<d.length; i++) {
		id = 'pwm-'+i;
		t = document.createElement('tr');
		t.className = i%2 ? '' : 'odd';
		table.appendChild(t);
		t.innerHTML = '<td width="120">Kana³ #'+(i+1)+'</td><td id="'+id+'" width="200">'+d[i]+'</td>' +
			'<td><form onsubmit="return set_pwm($(\''+id+'-val\'))"><input type="text" value="'+d[i]+'" id="'+id+'-val" onclick="this.select()"/></form></td>' + 
			'<td class="bar"><div id="'+id+'-bar"></div></td>';
		pwm_channels++;
	}

	setInterval('update_pwm()', 2000);
});}