var board_num = [];
var can_place = true;
var info = undefined;
var pos_x = undefined, pos_y = undefined;
var click_x = undefined, click_y = undefined, clicked = false;

function init() {
  var board = document.getElementById('board');
  if (board.getContext) {
    var ctx = board.getContext('2d');
    ctx.strokeStyle = 'rgb(102, 102, 102)';
    for (var i=0; i<18; ++i) {
      ctx.lineWidth = i % 9 == 0 ? 1.5 : 1;
      ctx.beginPath();
      ctx.moveTo(0, i * 35 + 17.5);
      ctx.lineTo(630, i * 35 + 17.5);
      ctx.stroke();
      ctx.beginPath();
      ctx.moveTo(i * 35 + 17.5, 0);
      ctx.lineTo(i * 35 + 17.5, 630);
      ctx.stroke();
    }
    board.setAttribute('style', 'clip-path: inset(0px 315px 315px 0px); top: 0px; left: 0px;');
    board.setAttribute('onmousedown', 'mouse_click(event);');
    board.setAttribute('onmousemove', 'mouse_move(event);');
    board.setAttribute('onmouseup', 'mouse_up(event);');
    pos_x = pos_y = 0;
  } else {
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
  }
  board_num = [];
  for (var i=0; i<81; ++i) {
    board_num.push(0);
  }
  info = document.getElementById('info');
  info.innerHTML = '로드되었습니다.';
}

function mouse_click(evn) {
  click_x = Math.floor(evn.offsetX / 35) % 9;
  click_y = Math.floor(evn.offsetY / 35) % 9;
  clicked = true;
  console.log(evn);
}

function mouse_move(evn) {
  if (clicked) {
    click_x = click_y = undefined;
    pos_x -= evn.movementX;
    pos_y -= evn.movementY;
    if (pos_x < 0) {
      pos_x += 315;
    }
    if (pos_x >= 315) {
      pos_x -= 315;
    }
    if (pos_y < 0) {
      pos_y += 315;
    }
    if (pos_y >= 315) {
      pos_y -= 315;
    }
    var style = 'top: ' + (-pos_y) + 'px; left: ' + (-pos_x) + 'px; ';
    style += 'clip-path: inset(' + pos_y + 'px ' + (315 - pos_x) + 'px ';
    style += (315 - pos_y) + 'px ' + pos_x + 'px);';
    board.setAttribute('style', style);
  }
}

function mouse_up(evn) {
  clicked = false;
  console.log(click_x, click_y);
  if (click_x !== undefined && click_y !== undefined) {
    pressed(click_y, click_x);
  }
}

function put_action(y, x, is_opponent) {
  if (pos_x !== undefined) {
    var ctx = document.getElementById('board').getContext('2d');
    ctx.fillStyle =  is_opponent ? '#f66' : '#090';
    for (var dx=0; dx<=2; ++dx) {
      for (var dy=0; dy<=2; ++dy) {
        ctx.beginPath();
        ctx.arc(x * 35 + dx * 315 + 17, y * 35 + dy * 315 + 17, 15, 0, 2 * Math.PI, false);
        ctx.fill();
      }
    }
  } else {
    var caller = document.getElementById('board-' + y + '-' + x);
    caller.classList.add(is_opponent ? 'board-red' : 'board-green');
  }
}

function pressed(y, x) {
  if (can_place && board_num[y * 9 + x] === 0) {
    can_place = false;
    info.innerHTML = 'AI가 생각하는 중입니다...';
    put_action(y, x, false);
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
          put_action(y, x, true);
          board_num[pos] = 1;
          can_place = true;
        }
        var game_status = +arr[1];
        if (game_status >= 0) {
          can_place = false;
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
