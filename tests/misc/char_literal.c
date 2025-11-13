
int main(){
  assert(65, 'A', "\'A\'");
  assert(97, 'a', "\'a\'");
  assert(92, '\\', "\'\\\\\'");
  assert(39, '\'', "\'\\\'\'");
  assert(34, '\"', "\'\\\"\'");
  assert(10, '\n', "\'\\n\'");
  assert(13, '\r', "\'\\r\'");
  assert(9, '\t', "\'\\t\'");
  assert(11, '\v', "\'\\v\'");
  assert(8, '\b', "\'\\b\'");
  assert(12, '\f', "\'\\f\'");
  assert(7, '\a', "\'\\a\'");
  assert(63, '\?', "\'\\?\'");
  assert(0, '\0', "\'\\0\'");
  
  return 0;
}
