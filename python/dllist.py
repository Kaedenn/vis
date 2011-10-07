#!/usr/bin/env python

"""Doubly-Linked List"""

class _dlitem(object):
  def __init__(self, value, prev = None, next = None):
    self._value = value
    self._prev = prev
    self._next = next
  
  def link(self, prev, next):
    self._prev = prev
    self._next = next
  
  def prev(self): return self._prev
  def next(self): return self._next
  def value(self): return self._value
  
  def removeself(self):
    if self._prev is not None:
      self._prev._next = self._next
    if self._next is not None:
      self._next._prev = self._prev

def _link(item1, item2):
  item1.link(item1.prev(), item2)
  item2.link(item1, item2.next())

class dllist(object):
  def __init__(self, *values):
    self._head = None
    self._tail = None
    for value in values:
      self.pushback(value)
  
  def __iter__(self):
    curr = self._head
    while curr is not None:
      next = curr.next()
      yield curr
      curr = next
  
  def remove(self, item):
    if item is self._head:
      self.popfront()
    elif item is self._tail:
      self.popback()
    else:
      item.removeself()
  
  def clear(self):
    self._head = self._tail = None
  
  def pushback(self, value):
    value = _dlitem(value)
    if self._tail is None:
      self._head = self._tail = value
    else:
      _link(self._tail, value)
      self._tail = value
  
  def pushfront(self, value):
    value = _dlitem(value)
    if self._head is None:
      self._head = self._tail = value
    else:
      _link(value, self._head)
      self._head = value
  
  def popback(self):
    if self._tail is None:
      raise IndexError("pop from an empty dllist")
    tail = self._tail
    self._tail = self._tail.prev()
    tail.removeself()
    if self._tail is None:
      self._head = None
  
  def popfront(self):
    if self._head is None:
      raise IndexError("pop from an empty dllist")
    head = self._head
    self._head = self._head.next()
    head.removeself()
    if self._head is None:
      self._tail = None

