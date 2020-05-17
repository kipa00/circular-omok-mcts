var board_num = [];
var can_place = true;
var info = undefined;

function init() {
  var board = document.getElementById('board');
  for (var i=0; i<9; ++i) {
    var tr_elem = document.createElement('tr');
    for (var j=0; j<9; ++j) {
      var elem = document.createElement('td');
      elem.setAttribute('id', 'board-' + i + '-' + j);
      elem.setAttribute('onclick', 'pressed(' + i + ',' + j + ');');
      tr_elem.appendChild(elem);
    }
    board.appendChild(tr_elem);
  }
  board_num = [];
  for (var i=0; i<81; ++i) {
    board_num.push(0);
  }
  info = document.getElementById('info');
  info.innerHTML = '로드되었습니다.';
}

function pressed(y, x) {
  if (can_place && board_num[y * 9 + x] === 0) {
    can_place = false;
    info.innerHTML = 'AI가 생각하는 중입니다...';
    var caller = document.getElementById('board-' + y + '-' + x);
    caller.classList.add('board-green');
    board_num[y * 9 + x] = 2;
    get_next();
  }
}

function get_next() {
  var s = board_num.join('');
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (xhr.readyState === xhr.DONE) {
      if (xhr.status === 200 || xhr.status === 201) {
        var arr = xhr.responseText.split(' ');
        var pos = +arr[0];
        var y = Math.floor(pos / 9), x = pos % 9;
        if (pos !== -1) {
          document.getElementById('board-' + y + '-' + x).classList.add('board-red');
          board_num[pos] = 1;
          can_place = true;
        }
        var game_status = +arr[1];
        if (game_status >= 0) {
          if (game_status === 0) {
            info.innerHTML = '비겼습니다.';
          } else if (game_status === 1) {
            info.innerHTML = '졌습니다.';
          } else if (game_status === 2) {
            info.innerHTML = '이겼습니다.';
          }
          info.innerHTML += ' 새 판을 플레이하시려면 새로고침해 주세요.';
        } else {
          info.innerHTML = '돌이 ' + String.fromCharCode(y + 65) + (x + 1) + ' 위치에 놓였습니다.';
        }
      } else {
        info.innerHTML = '서버에 접속할 수 없습니다. 잠시 후 다시 시도해 주세요.';
      }
    }
  };
  xhr.open('GET', 'http://localhost:9450/' + s);
  xhr.send();
}
