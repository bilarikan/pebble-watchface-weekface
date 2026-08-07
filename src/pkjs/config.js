var MONTHS = ['January', 'February', 'March', 'April', 'May', 'June',
              'July', 'August', 'September', 'October', 'November', 'December'];
var MONTHS_ABBR = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
                   'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
var DAYS = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
var DAYS_ABBR = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

function nameInputs(keyBase, defaults, limit) {
  return defaults.map(function(name, i) {
    return {
      type: 'input',
      messageKey: keyBase + '[' + i + ']',
      label: name,
      defaultValue: name,
      attributes: { limit: limit }
    };
  });
}

module.exports = [
  {
    type: 'heading',
    defaultValue: 'Weekface Settings'
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Time'
      },
      {
        type: 'radiogroup',
        messageKey: 'TimeFormat',
        label: 'Time format',
        defaultValue: '1',
        options: [
          { label: '12-hour (1:05)', value: '0' },
          { label: '24-hour (13:05)', value: '1' },
          { label: 'Military (1305)', value: '2' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Display'
      },
      {
        type: 'radiogroup',
        messageKey: 'WeekdayStyle',
        label: 'Weekday in date line',
        defaultValue: '0',
        options: [
          { label: 'Full (Friday)', value: '0' },
          { label: 'Abbreviated (Fri)', value: '1' },
          { label: 'Hidden', value: '2' }
        ]
      },
      {
        type: 'radiogroup',
        messageKey: 'MonthStyle',
        label: 'Month in date line',
        defaultValue: '0',
        options: [
          { label: 'Full (August)', value: '0' },
          { label: 'Abbreviated (Aug)', value: '1' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'ShowWeek',
        label: 'Week (W32)',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowQuarter',
        label: 'Quarter-end week (Q39)',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowDay',
        label: 'Day of year (D219)',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowRemaining',
        label: 'Days remaining in year (R146)',
        defaultValue: true
      }
    ]
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Vibration'
      },
      {
        type: 'toggle',
        messageKey: 'HourlyVibe',
        label: 'Vibrate on the hour',
        defaultValue: false
      },
      {
        type: 'toggle',
        messageKey: 'ConnVibe',
        label: 'Vibrate on connect / disconnect',
        defaultValue: false
      }
    ]
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Translations'
      },
      {
        type: 'text',
        defaultValue: 'Rename weekdays and months in your language. ' +
                      'Abbreviated weekday names also label the calendar columns ' +
                      '(first two letters). Leave a field unchanged to keep the default.'
      },
      { type: 'heading', size: 5, defaultValue: 'Indicator letters' },
      {
        type: 'input',
        messageKey: 'IndLetters[0]',
        label: 'Week (W in W32)',
        defaultValue: 'W',
        attributes: { limit: 3 }
      },
      {
        type: 'input',
        messageKey: 'IndLetters[1]',
        label: 'Quarter end (Q in Q39)',
        defaultValue: 'Q',
        attributes: { limit: 3 }
      },
      {
        type: 'input',
        messageKey: 'IndLetters[2]',
        label: 'Day of year (D in D219)',
        defaultValue: 'D',
        attributes: { limit: 3 }
      },
      {
        type: 'input',
        messageKey: 'IndLetters[3]',
        label: 'Days remaining (R in R146)',
        defaultValue: 'R',
        attributes: { limit: 3 }
      },
      { type: 'heading', size: 5, defaultValue: 'Weekdays' }
    ].concat(nameInputs('DaysFull', DAYS, 11), [
      { type: 'heading', size: 5, defaultValue: 'Weekdays, abbreviated' }
    ], nameInputs('DaysAbbr', DAYS_ABBR, 5), [
      { type: 'heading', size: 5, defaultValue: 'Months' }
    ], nameInputs('MonthsFull', MONTHS, 11), [
      { type: 'heading', size: 5, defaultValue: 'Months, abbreviated' }
    ], nameInputs('MonthsAbbr', MONTHS_ABBR, 5))
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
